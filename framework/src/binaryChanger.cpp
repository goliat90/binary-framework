/*  Handles the growth or shrinkage of a binary.
    Rewrites addresses.
    Corrects branch addresses.
    Symbol table entries fixed.
    Segment sizes changed.
*/

/*  Steps that need to be done.
    4. extend an existing ELF segment or add a new one
    5. move later segments to higher addresses
    1. change the addresses for the subsequent instructions.
    2. fix up target addresses for branch instructions
    3. fix symbol tables to point to new addresses
    6. fix the ELF segment table
*/

/*  Achieve the steps above i need to analyze the binary
    before applying transform. Basically get a state
    of the original binary. After transforms has been applied
    the new segment sizes are calculated. Last is to
    apply the address rewriting steps. In summary.
    
    1. Analyze the binary before transformation to get
    all needed information to perform changes correctly.

    2. After transforms calculate the new segment sizes.

    3. Determine if a segment can grow in the current place.
    If not then move it to a new space. Start with the segment
    that has grown the most, then the one after that.

    3. Rewrite the addresses of instructions, branches,
    fix symbol tables.
*/

/*  Include. */
#include "binaryChanger.hpp"


/*  Constructor. */
binaryChanger::binaryChanger(CFGhandler* cfgH, SgProject* passedProjectPtr) {
    /*  Save the cfg handler pointer. */
    cfgObject = cfgH;
    /*  Save the project pointer. */
    changerProjectPtr = passedProjectPtr;
    /*  set debugging to false as default. */
    debugging = false;
}


/*  This function performs analysis on the binary before
    the transformations are applied. */
void binaryChanger::preTransformationAnalysis() {
    /*  Collect needed data and structures for sections, segments etc. */
    preSegmentSectionCollection();

    /*  Analyze all the basic blocks and collect information about them. */
    preBlockInformationCollection();

    /*  Collect information on which target address blocks has. */
    /*  Has to be collected as some branches are connected to their target
        instruction, when it moved the target changes. */
    preBranchAnalysis();
}


/*  Post tranformation function. Fixes the binary so it
    is fixed. */
void binaryChanger::postTransformationWork() {
    /*  Check how the blocks have changed in size. */
    postChanges();

    /*  Reallocate segments. */
    reallocateSegments();

    /*  Segments have now been moved if they needed to be and
        their mapped size fixed to include the changes.
        Now the basic blocks address will be moved to the new address space.*/
    moveSegmentBasicBlocks();

    /*  After the segments have their basic blocks moved then
        go through all basic blocks and check all jump instructions
        and correct their jump target to the new address space. */
    redirectBranchInstructions();


    /*  At this point segments have been moved, their basic blocks as well,
        branches have been corrected. However the symboltable is possibly
        incorrect since it will still contain old addresses. It needs to be
        checked and adjusted so instructions such as jalr are correct. */
    correctSymbolTableFunctionEntries();

    /*  The physical file offsets need to be fixed, since some segments
        have grown the physical size of it needs to be fixed. Then
        all preceding segments needs to have their offset fixed. */
    fixSectionOffsets();
}


/*  Collect information about basic blocks that i need before any
    transformations are applied. */
void binaryChanger::preBlockInformationCollection() {
    /*  Get the program cfg pointer. */
    CFG* entireCFG = cfgObject->getProgramCFG();
    /*  Go through the program CFG and extract all the basic blocks. */
    for(std::pair<CFGVIter, CFGVIter> iterPair = vertices(*entireCFG);
        iterPair.first != iterPair.second; ++iterPair.first) {
        /*  Extract the basic block. */
        SgAsmBlock* nodeBlock = get(boost::vertex_name, *entireCFG, *iterPair.first);
        /*  Store the block in the vector. */
        basicBlockVector.push_back(nodeBlock);
        /*  Retrieve the size of the basic block, that is the
            number of instructions in the block. */
        SgAsmStatementPtrList& stmtList = nodeBlock->get_statementList();
        int blockSize = stmtList.size();
        blockOriginalSize.insert(std::pair<SgAsmBlock*, int>(nodeBlock, blockSize));

        /*  Get the address of the block and store it. */
        blockStartAddrMap.left.insert(std::pair<SgAsmBlock*, rose_addr_t>(nodeBlock, nodeBlock->get_id()));
        /*  Get the address of the last instruction in the block and store it. */
        rose_addr_t lastAddress;
        if (!stmtList.empty()) {
            /*  There are statements so get the last one. */
            SgAsmStatement* lastStatement = stmtList.back();
            lastAddress = lastStatement->get_address();
        } else {
            /*  The block is empty. The last address is then the first address. */
            if (debugging) {
                std::cout << "Found a block with no instructions." << std::endl;
                lastAddress = nodeBlock->get_id();
            }
        }
        blockEndAddrMap.insert(std::pair<SgAsmBlock*, rose_addr_t>(nodeBlock, lastAddress));

        /*  Debug print. */
//        if (debugging) {
//            std::cout << "Block with start address: "
//                << std::showbase << std::hex << nodeBlock->get_id() << std::endl
//                << "last address: " << std::hex << lastAddress << std::endl
//                << "size: " << std::dec << blockSize << std::endl
//                << "stored." << std::endl;
//        }
    }
    if (debugging) {
        std::cout << "Information about " << basicBlockVector.size() << " blocks collected." << std::endl;
    }

    /*  All the basic blocks have been inserted. Sort the 
        vector according to blocks address size. */
    std::sort(basicBlockVector.begin(), basicBlockVector.end(), blockSortStruct());
}


/*  Collect information about section and segments in the binary
    before any transformations are applied. */
void binaryChanger::preSegmentSectionCollection() {

    /*  Retrieve the sections. */
    elfSections = SageInterface::querySubTree<SgAsmElfSection>(changerProjectPtr);
    if (debugging) {
        std::cout << "elfs found: " << elfSections.size() << std::endl;
    }

    for(std::vector<SgAsmElfSection*>::iterator elfIter = elfSections.begin();
        elfIter != elfSections.end(); ++elfIter) {
        /* Check the reason of the section. */
        SgAsmElfSection::SectionPurpose pur = (*elfIter)->get_purpose();


        switch((*elfIter)->get_purpose()) {
            case SgAsmElfSection::SP_UNSPECIFIED:
                //std::cout << "Unknown elf" << std::endl;
                break;
            case SgAsmElfSection::SP_PROGRAM:
                /*  Add all the segments. */
                //TODO or add all the segments that are read and writable. then check the boundaries.
                //TODO if i add just those segment i could then use the sections addresses as boundaries.
                //segmentVector.push_back(*elfIter);
                break;
            case SgAsmElfSection::SP_HEADER:
                /*  Header Check here if the section is Read and Executable. */
                if (true == (*elfIter)->get_mapped_rperm() && true == (*elfIter)->get_mapped_xperm()) {
                    /*  The section is relevant, save it. */
                    sectionVector.push_back(*elfIter);
                    /*  Remember the address of this segment to get a lower
                        address boundary which i can not move segments below.
                        This has the limitation if i have more executable sections it will be incorrect. */
                    lowerVirtualAddressLimit = (*elfIter)->get_mapped_preferred_rva();
                }
                /*  Check if the header if Read and Writable. */
                if (true == (*elfIter)->get_mapped_rperm() && true == (*elfIter)->get_mapped_wperm()) {
                    /*  The section is relevant, save it. */
                    sectionVector.push_back(*elfIter);
                    /*  Save the base address for this section. It will be used
                        as a upper limit. This is assuming that this section is
                        after the code segment and there is only one data segment. */
                    upperVirtualAddressLimit = (*elfIter)->get_mapped_preferred_rva();
                }
                break;
            case SgAsmElfSection::SP_SYMTAB:
                //std::cout << "symbol table" << std::endl;
                break;
            case SgAsmElfSection::SP_OTHER:
                //std::cout << "file specified purpose than other categories." << std::endl;
                //TODO consider checking here if the segment is mapped it might be
                //TODO to add it to the segmentVector list.
                //TODO or later when i find the boundary for the .text i can go thorugh segments and
                //TODO and add all sections relevant...
                break;
            default:
                break;
        }

        /*  Debug printout of all sgasmsections. */
        if (debugging) {
            switch((*elfIter)->get_purpose()) {
                case SgAsmElfSection::SP_UNSPECIFIED:
                    std::cout << "Unknown elf" << std::endl;
                    break;
                case SgAsmElfSection::SP_PROGRAM:
                    std::cout << "Program-supplied, code, data etc." << std::endl;
                    break;
                case SgAsmElfSection::SP_HEADER:
                    std::cout << "header for executable format" << std::endl;
                    break;
                case SgAsmElfSection::SP_SYMTAB:
                    std::cout << "symbol table" << std::endl;
                    break;
                case SgAsmElfSection::SP_OTHER:
                    std::cout << "file specified purpose than other categories." << std::endl;
                    break;
                default:
                    std::cout << "unknown purpose enum." << std::endl;
                    break;
            }   
            /*  Extract the name of the segment. */
            SgAsmGenericString* elfString = (*elfIter)->get_name();
            /*  Get the string name. */
            std::cout << "Name: " << elfString->get_string() << std::endl;
            /*  Print flags of the section. */
            if ((*elfIter)->get_mapped_rperm()) {
                std::cout << "Readable." << std::endl;
            }   
            if ((*elfIter)->get_mapped_wperm()) {
                std::cout << "Writable." << std::endl;
            }   
            if ((*elfIter)->get_mapped_xperm()) {
                std::cout << "Executable." << std::endl;
            }
            /* Check if it should be mapped. */
            std::cout << "Is mapped: " << std::boolalpha << (*elfIter)->is_mapped() << std::endl;
            /* print base address of section. */
            std::cout << "Address (mapped_preferred_rva): " << std::hex << (*elfIter)->get_mapped_preferred_rva() << std::endl;
            /* size of section. */
            std::cout << "Size (mapped): " << std::hex << (*elfIter)->get_mapped_size() << std::endl;
            std::cout << "Size (file)  : " << std::hex << (*elfIter)->get_size() << std::endl;
            /*  offsets. */
            std::cout << "Offset(file) : " << std::hex << (*elfIter)->get_offset() << std::endl;

            std::cout << std::endl;
        }
    }

    /* Go through the elf sections and add any segment that is whithin the address range. */
    for(asmElfVector::iterator elfIter = elfSections.begin();
        elfIter != elfSections.end(); ++elfIter) {
        /*  For each section check its address (virtual). */
        rose_addr_t elfVa = (*elfIter)->get_mapped_preferred_rva();
        /*  See if it should be added or not. */
        if (lowerVirtualAddressLimit <= elfVa && elfVa < upperVirtualAddressLimit) {
            /* The section is within the bound, check so it is not the
                section itself by searching the sectionVector. */
            if (sectionVector.end() == std::find(sectionVector.begin(), 
                    sectionVector.end(), (*elfIter))) {
                /*  The elf section is not a section itself. */
                segmentVector.push_back(*elfIter);
            }
        }
    }

    /*  Sort the section according to their virtual addresses. */
    std::sort(sectionVector.begin(), sectionVector.end(), elfSectionSortStruct());

    /*  Sort the segments according to their virtual addresses. */
    std::sort(segmentVector.begin(), segmentVector.end(), elfSectionSortStruct());

    /*  Print if debugging. */
    if (debugging) {
        /* Print out content in the section vector. */
        std::cout << "---- Sections ----" << std::endl;
        for(asmElfVector::iterator elfIter = sectionVector.begin();
            elfIter != sectionVector.end(); ++elfIter) {
            /*  Extract the name of the segment. */
            SgAsmGenericString* elfString = (*elfIter)->get_name();
            /*  Get the string name. */
            std::cout << "Name: " << elfString->get_string() << std::endl;
            /*  Print flags of the section. */
            if ((*elfIter)->get_mapped_rperm()) {
                std::cout << "Readable." << std::endl;
            }   
            if ((*elfIter)->get_mapped_wperm()) {
                std::cout << "Writable." << std::endl;
            }   
            if ((*elfIter)->get_mapped_xperm()) {
                std::cout << "Executable." << std::endl;
            }
            /* Check if it should be mapped. */
            std::cout << "Is mapped: " << std::boolalpha << (*elfIter)->is_mapped() << std::endl;
            /* print base address of section. */
            std::cout << "Address (mapped_preferred_rva): " << std::hex << (*elfIter)->get_mapped_preferred_rva() << std::endl;
            /* size of section. */
            std::cout << "Size (mapped)      : " << std::hex << (*elfIter)->get_mapped_size() << std::endl;
            std::cout << "Size (file)        : " << std::hex << (*elfIter)->get_size() << std::endl;
            /*  Alignment of section in file. */
            std::cout << "alignment (mapped ): " << std::hex << (*elfIter)->get_mapped_alignment() << std::endl;
            std::cout << "alignment (file)   : " << std::hex << (*elfIter)->get_file_alignment() << std::endl;
            /*  offsets. */
            std::cout << "Offset(file)    : " << std::hex << (*elfIter)->get_offset() << std::endl;

            std::cout << std::endl;
        }
        std::cout << "---- End Sections ----" << std::endl << std::endl
            << "---- Segments ----" << std::endl;

        /*  Printout the segments that are within the virtual address boundary. */
        for(asmElfVector::iterator elfIter = segmentVector.begin();
            elfIter != segmentVector.end(); ++elfIter) {
            /*  Extract the name of the segment. */
            SgAsmGenericString* elfString = (*elfIter)->get_name();
            /*  Get the string name. */
            std::cout << "Name: " << elfString->get_string() << std::endl;
            /*  Print flags of the section. */
            if ((*elfIter)->get_mapped_rperm()) {
                std::cout << "Readable." << std::endl;
            }   
            if ((*elfIter)->get_mapped_wperm()) {
                std::cout << "Writable." << std::endl;
            }   
            if ((*elfIter)->get_mapped_xperm()) {
                std::cout << "Executable." << std::endl;
            }
            /* Check if it should be mapped. */
            std::cout << "Is mapped: " << std::boolalpha << (*elfIter)->is_mapped() << std::endl;
            /* print base address of section. */
            std::cout << "Address (mapped_preferred_rva): " << std::hex << (*elfIter)->get_mapped_preferred_rva() << std::endl;
            /* size of section. */
            std::cout << "Size (mapped)      : " << std::hex << (*elfIter)->get_mapped_size() << std::endl;
            std::cout << "Size (file)        : " << std::hex << (*elfIter)->get_size() << std::endl;
            /*  mapped alignment */
            std::cout << "alignment (mapped ): " << std::hex << (*elfIter)->get_mapped_alignment() << std::endl;
            /*  offsets. */
            std::cout << "Offset(file)       : " << std::hex << (*elfIter)->get_offset() << std::endl;

            std::cout << std::endl;
            
        }
        std::cout << "---- End Segments ----" << std::endl << std::endl;
    }
}


/*  Analyze branches and their targets. The information collected here
    is used to after transformations to fix the branches. */
void binaryChanger::preBranchAnalysis(){
    /*  Go through all the basic blocks and find all blocks
        with branching instructions. */

    /*  For each block check for a branching instruction. */
    for(std::vector<SgAsmBlock*>::iterator blockIter = basicBlockVector.begin();
        blockIter != basicBlockVector.end(); ++blockIter) {
        /*  get the statement list for the block. */
        SgAsmStatementPtrList& stmtList = (*blockIter)->get_statementList();
        /*  Bool to exit statement iteration when branch/jump is found. */
        bool exitStmtIter = false;
        /*  Iterate over it backwards until the first jump instruction is found. */
        for(SgAsmStatementPtrList::reverse_iterator revStmtIter = stmtList.rbegin();
            revStmtIter != stmtList.rend(); ++revStmtIter) {
            /*  Iterate until i encounter a branch or jump instruction. */
            SgAsmMipsInstruction* mips = isSgAsmMipsInstruction(*revStmtIter);
            /*  Check if the enum kind is of a desired instruction.
                For these instructions change the target address. */
            switch(mips->get_kind()) {
                case mips_beq:
                case mips_bne:
                
                case mips_bgez:
                case mips_bgezal:
                case mips_bgtz:
                case mips_blez:
                case mips_bltz:

                case mips_j:
                case mips_jal:
                case mips_jr:
                case mips_jalr: {
                    /*  Get the operands. */
                    SgAsmExpressionPtrList& opList = mips->get_operandList()->get_operands();
                    /*  Go through the operands and get the instruction constant. */
                    for(SgAsmExpressionPtrList::iterator opIter = opList.begin();
                        opIter != opList.end(); ++opIter) {
                        /*  Check if the operand is the target address. */
                        if (V_SgAsmIntegerValueExpression == (*opIter)->variantT()) {
                            /*  Cast pointer. */
                            SgAsmIntegerValueExpression* valueExpr = isSgAsmIntegerValueExpression(*opIter);
                            /*  It is the target address. Of the instruction. */
                            rose_addr_t target = valueExpr->get_absoluteValue();
                            /*  Check which basic block the branch jumps to. */
                            if (1 == blockStartAddrMap.right.count(target)) {
                                /*  The target address is a basic block, the the block pointer. */
                                SgAsmBlock* targetBlock = blockStartAddrMap.right.find(target)->second;
                                /*  Save the branch instruction and its target block in the map. */
                                branchTargetMap.insert(std::pair<SgAsmMipsInstruction*, SgAsmBlock*>(mips, targetBlock));
                                /* When the blocks branch/jump has been found break the iteration. */
                                exitStmtIter = true;
                            }
                        }
                    }
                }
            }
            /*  Check if the statement iteration should end. */
            if (true == exitStmtIter) {
                /*  break stmt for loop. */
                break;
            }
        }
    }
}


/*  Check how the transformatins have affected the blocks. */
void binaryChanger::postChanges() {
    /*  Go through the block vector and check the size changes. */
    for(std::vector<SgAsmBlock*>::iterator blockIter = basicBlockVector.begin();
        blockIter != basicBlockVector.end(); ++blockIter) {
        /*  get the original size. */
        int orgSize = blockOriginalSize.find(*blockIter)->second;
        /*  Get the potential new size. */
        SgAsmStatementPtrList& stmtList = (*blockIter)->get_statementList();
        int newSize = stmtList.size();
        /*  Check the size of all the blocks, if they have changed
            save the difference. */
        if(orgSize != newSize) {
            /*  save the new size. */
            blockSizeDifference.insert(std::pair<SgAsmBlock*, int>((*blockIter), (newSize - orgSize)));
        }
    }

    /*  Go through the segments and for each one determine which basic
        blocks belongs to it and check if their size have change.
        If so remember that the segment has changed it size also. */
    for(asmElfVector::iterator segIter = segmentVector.begin();
        segIter != segmentVector.end(); ++segIter) {
        /*  Get the virtual address of the segment and the size.
            Use that to determine the range of the segment.
            With that check which blocks are in the segment. */
        bool segmentModified = false;
        rose_addr_t segDiff = 0;
        rose_addr_t segAddr = (*segIter)->get_mapped_preferred_rva();
        rose_addr_t segMappedSize = (*segIter)->get_mapped_size();
        rose_addr_t segEndAddr = segAddr + segMappedSize;

        /*  Iterate over the blocks and check their address, if they belong
            in the segment then check if their size has changed. */
        for(std::vector<SgAsmBlock*>::iterator blockIter = basicBlockVector.begin();
            blockIter != basicBlockVector.end(); ++blockIter) {
            /*  Get the blocks start address. */
            rose_addr_t blockAddr = (*blockIter)->get_id();
            /*  Check if the address of the block is within the current segment. */
            if (segAddr <= blockAddr && blockAddr <= segEndAddr) {
                /* Block is within the segment, Check if it has a new size. */
                if (1 == blockSizeDifference.count(*blockIter)) {
                    /*  Add the difference to the segDiff. */
                    int blockDiff = blockSizeDifference.find(*blockIter)->second;
                    segDiff += blockDiff;
                    /*  Set boolean to true. */
                    segmentModified = true;
                }
            }
        }
        /*  If the segment size has changed then save the difference. */
        if (true == segmentModified) {
    //        segmentSizeDifference.insert(std::pair<SgAsmElfSection*, rose_addr_t>(*segIter, segDiff));
            segDiff *= 4;
            segmentSizeDifference.left.insert(std::pair<SgAsmElfSection*, rose_addr_t>(*segIter, segDiff));
            /*  Printout debugging information. */
            if (debugging) {
                /*  Extract the name of the segment. */
                SgAsmGenericString* elfString = (*segIter)->get_name();
                /*  Get the string name. */
                std::cout << "Name: " << elfString->get_string()
                    << " Size has changed with " << segDiff << " bytes." << std::endl;
            }
        }
    }
}


/*  Start moving segments that can not grow in its current place.
    The largest segment will be moved first, then continue with
    the second largest, e.t.c. If this function can't find a
    place for a segment the transformation fails. */
void binaryChanger::reallocateSegments() {
    /*  Find all segments that have been changed and add them to
        some kind of container. When a segment has is determined
        to be in a space it can be in then it is removed from the
        container. Continue until the container is empty. */
    std::vector<rose_addr_t> modifiedElfSections;
    for(segDiffType::left_iterator leftSegIter = segmentSizeDifference.left.begin();
        leftSegIter != segmentSizeDifference.left.end(); ++leftSegIter) {
        /*  Copy over the elfsection to the set. */
        modifiedElfSections.push_back(leftSegIter->second);
    }
    /*  Sort it small to large. */
    std::sort(modifiedElfSections.begin(), modifiedElfSections.end(), std::greater<rose_addr_t>());

    /*  While there still are not controlled elfsections continue
        checking them. */
    while(!modifiedElfSections.empty()) {
        /*  Find all the current free address spaces. */
        findFreeVirtualSpace();
        /*  Start with the segment that has grown the most, check if it
            can remain in its current position or if it has to be moved. */
        rose_addr_t neededSegSpace = modifiedElfSections.back();
        //TODO if two segments have the same required space this approach wont work.
        SgAsmElfSection* checkedSegment = segmentSizeDifference.right.find(neededSegSpace)->second;
        /*  Size of the checked segment. */
        rose_addr_t segSize = checkedSegment->get_mapped_size();

        if (debugging) {
            /*  Extract the name of the segment. */
            SgAsmGenericString* elfString = checkedSegment->get_name();
            std::cout << "Checking if " << elfString->get_string() 
                << " needs to be moved." << std::endl;
        }

        /*  Check if the segment has some space after it and if it is
            enough to fit the changes to the segment. */
        if (1 == addressVoids.right.count(checkedSegment)) {
            /*  The segment has space after it, check if it is enough. */
            rose_addr_t addrSpace = addressVoids.right.find(checkedSegment)->second;
            /*  Check if the available space is enough. */
            if (neededSegSpace <= addrSpace) {
                /*  The space is enough. */
                if (debugging) {
                    std::cout << "Segment can grow in place, no move needed." << std::endl;
                }
                /*  Adjust the segment size to include the changes. */
                rose_addr_t newSize =  + neededSegSpace;
                /*  Remove the segment from the list, done by poping it. */
                modifiedElfSections.pop_back();
                /*  Continue with the next segment that has changed. */
                continue;
            }
        }
        /*  Boolean is set if a new space is found. */
        bool segmentMoved = false;
        /*  New address for the segment. */
        rose_addr_t newAddress = 0;
        /*  If we reach here then the segment needs to be moved.
            Go through the free spaces and use the first one that fits. */
        for(addressVoidType::left_iterator addrVoidIter = addressVoids.left.begin();
            addrVoidIter != addressVoids.left.end(); ++addrVoidIter) {
            /*  Check if the space is large enough. It needs to be able to
                hold the original size plus the new space. Before checking
                i need to check the alignment, which might mean i need more
                space due to the alignment. */
            rose_addr_t requiredSpace = (segSize + neededSegSpace);
            /*  Segment that has space after it. */
            SgAsmElfSection* spaceSegment = addrVoidIter->second;
            /*  Calculate the new base address for the segment. */
            rose_addr_t spaceStartAddress = spaceSegment->get_mapped_preferred_rva() + spaceSegment->get_mapped_size();
            /*  Get the alignment of the segment that is being moved. */
            rose_addr_t mappedAlign = checkedSegment->get_mapped_alignment();
            /*  Calculate modulo on the start address, if is not aligned
                then add the missing amount to requiredSpace to get the
                segment aligned. */
            rose_addr_t missAlignment = spaceStartAddress % mappedAlign;
            /*  Check if the start address is not aligned. */
            if (0 != missAlignment) {
                /*  Include extra space for alignment. */
                requiredSpace += mappedAlign - missAlignment;
            }

            //TODO this is actually alignment of the mapped section, i can get that parameter.
            //TODO then modulo the address, if the result is zero then it is okay,
            //TODO otherwise add appropriate value to get the segment starting address aligned.
            //TODO looks like i need to adjust the address over with alignment.
            //TODO i need to determine the segSize with alignment included.

            if ((segSize + neededSegSpace) <= addrVoidIter->first) {
                /*  Found space after a segment that is large enough. */
//                SgAsmElfSection* segment = addrVoidIter->second;
//                /*  Calculate the new base address for the segment. */
                newAddress = spaceStartAddress + (mappedAlign - missAlignment);
                //TODO adjust here for alignment
                /*  Adjust the address so it is aligned correctly. */
                //newAddress += mappedAlign - missAlignment;
                /*  Set flag to true. */
                segmentMoved = true;
                /*  debugging. */
                if (debugging) {
                    SgAsmGenericString* elfString = spaceSegment->get_name();
                    std::cout << "Placing segment after segment " << elfString->get_string() << std::endl
                                << "New calculated address: " << std::hex << newAddress << std::endl;
                }
                /*  Break loop. */
                break;
            }
        }

        /*  Check if the segment was reallocated or it failed. */
        if (true == segmentMoved) {
            /*  Save the segments old address and size. Will be used to find its basic blocks. */
            segmentOldAddr.insert(std::pair<SgAsmElfSection*, rose_addr_t>
                                        (checkedSegment, checkedSegment->get_mapped_preferred_rva()));
            segmentOldSize.insert(std::pair<SgAsmElfSection*, rose_addr_t>
                                        (checkedSegment, checkedSegment->get_mapped_size()));
            /*  Set the new virtual address of the segment and set the new size of it.
                This is so when the next address voids are found it will not be incorrect. */
            checkedSegment->set_mapped_preferred_rva(newAddress);
            checkedSegment->set_mapped_size(segSize + neededSegSpace);

            if (debugging) {
                SgAsmGenericString* elfString = checkedSegment->get_name();
                std::cout << "Segment: " <<  elfString->get_string() << " moved." << std::endl
                    << "new address: " << std::hex << checkedSegment->get_mapped_preferred_rva() << std::endl
                    << "new size: " << std::hex << checkedSegment->get_mapped_size() << std::endl;
            }
        } else {
            /*  Failed to move segment so throw error. */
            SgAsmGenericString* elfString = checkedSegment->get_name();
            ASSERT_not_reachable("Unable to find new place for segment: " + elfString->get_string());
        }
        /*  Remove the segment from the list of modified segments since it
            is in a acceptable place. It is done by poping the back sector. */
        modifiedElfSections.pop_back();
    }

    /*  Print segments as debug. */
    if (debugging) {
        /*  Sort the segments before printing. */
        std::sort(segmentVector.begin(), segmentVector.end(), elfSectionSortStruct());
        std::cout << "---- Segments ----" << std::endl;
        /*  Printout the segments that are within the virtual address boundary. */
        for(asmElfVector::iterator elfIter = segmentVector.begin();
            elfIter != segmentVector.end(); ++elfIter) {
            /*  Extract the name of the segment. */
            SgAsmGenericString* elfString = (*elfIter)->get_name();
            /*  Get the string name. */
            std::cout << "Name: " << elfString->get_string() << std::endl;
            /*  Print flags of the section. */
            if ((*elfIter)->get_mapped_rperm()) {
                std::cout << "Readable." << std::endl;
            }
            if ((*elfIter)->get_mapped_wperm()) {
                std::cout << "Writable." << std::endl;
            }
            if ((*elfIter)->get_mapped_xperm()) {
                std::cout << "Executable." << std::endl;
            }
            /* Check if it should be mapped. */
            std::cout << "Is mapped: " << std::boolalpha << (*elfIter)->is_mapped() << std::endl;
            /* print base address of section. */
            std::cout << "Address (mapped_preferred_rva): " << std::hex << (*elfIter)->get_mapped_preferred_rva() << std::endl;
            /* size of section. */
            std::cout << "Size (mapped): " << std::hex << (*elfIter)->get_mapped_size() << std::endl;
            std::cout << "Size (file)  : " << std::hex << (*elfIter)->get_size() << std::endl;
            /*  offsets. */
            std::cout << "Offset(file) : " << std::hex << (*elfIter)->get_offset() << std::endl;

            std::cout << std::endl;

        }
        std::cout << "---- End Segments ----" << std::endl << std::endl;
    }
}

/*  Go through the address space and find the open spaces. */
void binaryChanger::findFreeVirtualSpace() {
    /*  Clearing address voids. */
    addressVoids.clear();

    /*  Sort the segment so they are in order before iterating over it. */
    std::sort(segmentVector.begin(), segmentVector.end(), elfSectionSortStruct());

    for(asmElfVector::iterator segIter = segmentVector.begin();
        segIter != segmentVector.end(); ++segIter) {
        /*  Get address and size of the segment. */
        rose_addr_t segAddr = (*segIter)->get_mapped_preferred_rva();
        rose_addr_t segSize = (*segIter)->get_mapped_size();
        /*  Calculate the end address of the segment. */
        rose_addr_t segEndAddr = segAddr + segSize;
        /*  Retrieve the next segments start address, or
            if it is the last segment then the next sections start address. */
        rose_addr_t nextSegAddr = 0;
        if (segmentVector.end() != (segIter+1)) {
            /*  It is not the last segment we are checking so
                there is another segment which an address can be retrieved from. */
            nextSegAddr = (*(segIter+1))->get_mapped_preferred_rva();
        } else {
            /*  It is the last segment in the section so we have to check
                the following sections start address. The address i use is
                therefore the upperVirtualAddressLimit variable. */
            nextSegAddr = upperVirtualAddressLimit;
        }
        /*  Check if the segments address plus size is equal to the next address. */
        if (nextSegAddr != (segAddr + segSize)) {
            /*  There is space available, make a map entry with the
                first segment and the space available. */
            rose_addr_t addrSpace = nextSegAddr - (segAddr + segSize);
            addressVoids.left.insert(std::pair<rose_addr_t, SgAsmElfSection*>(addrSpace, (*segIter)));
            /*  Debug print. */
            if (debugging) {
                /*  Get the string name. */
                SgAsmGenericString* elfString = (*segIter)->get_name();
                /*  Print name and space available. */
                std::cout << "Space found after segment " 
                    << elfString->get_string() << std::endl
                    << "Available space is " << std::hex << addrSpace << " bytes" << std::endl;
            }
        }
    }

        //TODO Go through the segment list and check between each segment
        //TODO if there is any space between them.
        //TODO first segment address + size,
        //TODO compare with the second segments address to see if there is any space there.

        //TODO special case is when im looking at the first segment and last in the list.
        //TODO with the last i need to check against the following section.
        //TODO with the first i might not need to care other than i do not allow anything to be moved above
        //TODO the first segments address.
}



/*  Moves basic blocks to the new location of the segment. It finds the basic
    blocks that are in that segment and assigns them new addresses in the new
    address space of the segment. */
//TODO what happens when two segments trade address space?
//TODO i need to consider this case, currently my code can probably not
//TODO handle that and will screw up things.
//TODO Fast solution is to probably use a copy of basicBlockVector and
//TODO remove blocks as they are identified.
void binaryChanger::moveSegmentBasicBlocks() {
    int blocksMoved = 0;
    /*  Make a local copy of the basicBlockVector, it is used to find
        blocks belonging to a segment in its old position. */
    std::vector<SgAsmBlock*> basicBlocks(basicBlockVector.begin(), basicBlockVector.end());
    /*  Go through all the modified segments and rewrite blocks and instructions
        addresses. */
    for(std::map<SgAsmElfSection*, rose_addr_t>::iterator segIter = segmentOldAddr.begin();
        segIter != segmentOldAddr.end(); ++segIter) {
        /*  Blocks that belong to the segment. */
        std::vector<SgAsmBlock*> segmentBlocks;
        /*  Old address of the segment and the old size of it. */
        rose_addr_t segOldAddr = segIter->second;
        rose_addr_t segOldSize = segmentOldSize.find(segIter->first)->second;
        rose_addr_t segOldEndAddr = segOldAddr + segOldSize;

        /*  Go through the basic block vector and find all blocks that belong to the
            Segment. Save them to iterater later. */
        //TODO need to change this to remove blocks that have been rewritte.
        for(std::vector<SgAsmBlock*>::iterator basicIter = basicBlocks.begin();
            basicIter != basicBlocks.end(); ++basicIter) {
            /*  Block address. */
            rose_addr_t blockAddr = (*basicIter)->get_id();
            /*  Check if the address of the block is within the current segment. */
            if (segOldAddr <= blockAddr && blockAddr <= segOldEndAddr) {
                /*  The block belongs to the segment so it will be moved.
                    Save it in the vector. */
                segmentBlocks.push_back(*basicIter);
            }
        }
        //TODO perhaps ensure that the vector is not empty?

        blocksMoved = segmentBlocks.size();

        /*  Go though the basic blocks that belong to the segment and
            rewrite the addresses. Blocks will be traversel in address
            order so the list is sorted first. */
        std::sort(segmentBlocks.begin(), segmentBlocks.end(), blockSortStruct());
        /*  Before iteration check if there is an address gap between the segments
            address and the first basic block, if there is preserve it. */
        SgAsmBlock* firstBlock = segmentBlocks.front();
        /*  Variables used to determine gaps between the basic blocks.
            They are used to preserve the gaps. between blocks. */
        rose_addr_t firstBlockAddr = firstBlock->get_id();

        /*  The first blocks address needs to be set before iteration in order
            to determine the gap between it and the segments start address. */
        if (segOldAddr < firstBlockAddr) {
            /*  There was a gap between segments first address and first block
                in the old address space, replicate it. */
            /*  Get the gap difference. */
            rose_addr_t newAddr = firstBlockAddr - segOldAddr;
            /*  Add the segment address. The blocks address will
                be the segment addres plus the gap. */
            newAddr = (segIter->first)->get_mapped_preferred_rva();
            /*  Set the address of the first block. */
            firstBlock->set_id(newAddr);
        } else if (segOldAddr == firstBlockAddr) {
            /*  The first basic block was at the first address so no gap.
                Do the same in the new address space. */
            firstBlock->set_id((segIter->first)->get_mapped_preferred_rva());
        }
        //TODO Need to make an entry regarding the basic blocks new addres,
        //TODO make a mapping between the new and old address, is for symboltable rewritting.

        /*  Debugging. Print address. */
        if (debugging) {
            std::cout << "First blocks address set to: " 
                << std::hex << firstBlock->get_id() << std::endl;
        }

        /*  Go through the basic blocks and start rewriting addresses.
            The address sequence is determined by checking the blocks address.
            The gap is determined to the next block and that block is assigned its address. */
        for(std::vector<SgAsmBlock*>::iterator segBlockIter = segmentBlocks.begin();
            segBlockIter != segmentBlocks.end(); ++segBlockIter) {
            /*  Get the current blocks id, which is the new one at this point */
            rose_addr_t blockAddr = (*segBlockIter)->get_id();
            /*  Get the blocks old starting address. */
            rose_addr_t oldBlockAddr = blockStartAddrMap.left.find(*segBlockIter)->second;
            /*  Create mapping between old and new address. */
            oldToNewAddrMap.insert(std::pair<rose_addr_t, rose_addr_t>(oldBlockAddr, blockAddr));

            /*  Get the blocks statement list. */
            SgAsmStatementPtrList& stmtList = (*segBlockIter)->get_statementList();

            /*  Iterate through the statement lists and rewrite the addresses.
                The first instruction will have the same address as basic block. */
            for(SgAsmStatementPtrList::iterator stmtIter = stmtList.begin();
                stmtIter != stmtList.end(); ++stmtIter) {
                /*  Assign the new address the instruction. */
                (*stmtIter)->set_address(blockAddr);
                /*  Increment the variable. */
                if (stmtList.end() != (stmtIter+1)) {
                    blockAddr += 4;
                }
            }
            /*  All instructions have had their address rewritten.
                Now to determine the address gap to the next basic block.
                If it is the last block then skip this process. */
            if (segmentBlocks.end() != (segBlockIter+1)) {
                /*  Get the blocks old last address. */
                rose_addr_t blockEndAddr = blockEndAddrMap.find(*segBlockIter)->second;
                /*  Get the next block and its start address. */
                SgAsmBlock* nextBlock = *(segBlockIter+1);
                rose_addr_t nextBlockAddr = blockStartAddrMap.left.find(nextBlock)->second;
                /*  Get the difference in addresses between the blocks. */
                rose_addr_t addrDiff = nextBlockAddr - blockEndAddr;
                /*  Add the gap to the address to replicate the gap. */
                blockAddr += addrDiff;
                /*  Assign the address to the next block, which will be traversed next. */
                nextBlock->set_id(blockAddr);
            }
            /*  Printout the basic blocks, to inspect addresses, */
            //printBasicBlockInstructions(*segBlockIter);
        }
        //TODO Take consideration that if there is some address space between the
        //TODO start of the segment and first basic blocks address.
    }
    /*  test to see that the number of basic blocks moved are the
        same as the amount of entries in old to new. */
    if (blocksMoved != oldToNewAddrMap.size()) {
        std::cout << "all moved blocks do not have a entry in oldToNewMapping" << std::endl
            << "blocksMoved = " << blocksMoved << std::endl
            << "oldtonewmap = " << oldToNewAddrMap.size() << std::endl;
    }
}


/*  Iterates through all the basic blocks and check for branch instructions.
    All branch instructions which has a new target address is exchanged
    for the new instructions address. */
void binaryChanger::redirectBranchInstructions() {
    
    //TODO since i have the map of branches and targets blocks.
    //TODO do i need search through all basic blocks then?
    //TODO i get the branch so i can check how the target is.
    //TODO i can inspect the block to see how it is related to the branch.
    //TODO as in, is the branch connected to an instruction, or set as a constant?


    /*  Check all the branches and their target blocks. */
    for(std::map<SgAsmMipsInstruction*, SgAsmBlock*>::iterator branchIter = branchTargetMap.begin();
        branchIter != branchTargetMap.end(); ++branchIter) {
        /*  Get the branch and get its target address. */
        SgAsmMipsInstruction* branchInst = branchIter->first;
        /*  Get the target basic block. */
        SgAsmBlock* targetBlock = branchIter->second;
        /*  Get the target blocks original address. */
        rose_addr_t blockOriginalAddress = blockStartAddrMap.left.find(targetBlock)->second;
        /*  Save the blocks new address. */
        rose_addr_t blockNewAddress = targetBlock->get_id();
        /*  Get the statement list of the block. */
        SgAsmStatementPtrList& blockStmtList = targetBlock->get_statementList();
        /*  Get the address of the last instruction in the block. */
        rose_addr_t blockNewEndAddress = blockStmtList.back()->get_address();

        /*  Retrieve the branch instructions target address. */
        //rose_addr_t targetAddr = 0;
        /*  Get the operands. */
        SgAsmExpressionPtrList& opList = branchInst->get_operandList()->get_operands();
        /*  Go through the operands and get the instruction constant. */
        for(SgAsmExpressionPtrList::iterator opIter = opList.begin();
            opIter != opList.end(); ++opIter) {
            /*  Check if the operand is the target address. */
            if (V_SgAsmIntegerValueExpression == (*opIter)->variantT()) {
                /*  Cast pointer. */
                SgAsmIntegerValueExpression* valueExpr = isSgAsmIntegerValueExpression(*opIter);
                /*  It is the target address. Of the instruction. */
                rose_addr_t targetAddr = valueExpr->get_absoluteValue();

                /*  Check if the target address is the original one but
                    the block has a new address. This means that the
                    branch target address is not mapped to a instruction in
                    the block. Just change the target in this case. */
                if ((targetAddr == blockOriginalAddress) && (blockOriginalAddress != blockNewAddress)) {
                    /*  Change the value to the new start address of the block. */
                    valueExpr->set_absoluteValue(blockNewAddress);
                    /*  debug print */
                    if (true) {
                        std::cout << "Constant branch target changed from " 
                            << std::hex << targetAddr << " to " << std::hex << blockNewAddress << std::endl;
                        printInstruction(branchInst);
                        //printBasicBlockInstructions(targetBlock);
                    }
                }
                /*  Check if the target address belongs to an instruction
                    withing the block. If true then the branch is relative so
                    point it to the first instruction in the block. */
                else if ((blockNewAddress < targetAddr) && (targetAddr <= blockNewEndAddress)) {
                    /*  Target is an instruction within the block but not the first.
                        The address of the target is linked to the branch target. */
                    SgAsmMipsInstruction* blockFirstInstruction = isSgAsmMipsInstruction(blockStmtList.front());
                    /*  Set the first instruction as the relative instruction
                        for the target offset for the branch instruction. */
                    valueExpr->makeRelativeTo(blockFirstInstruction);
                    /*  The absolute value is not changed by make relative to.
                        It only has a connection to the instruction. So set the absolute value. */
                    valueExpr->set_absoluteValue(blockFirstInstruction->get_address());
                    /*  Debug print. */
                    if (true) {
                        std::cout << "Relative branch target changed from " 
                            << targetAddr << " to " << std::hex << blockNewAddress << std::endl;
                            printInstruction(branchInst);
                            //printBasicBlockInstructions(targetBlock);
                        if (blockFirstInstruction == valueExpr->get_baseNode()) {
                            std::cout << "node was changed." << std::endl;
                        }
                    }
                }
            }
        }
        /*  Check if the blocks address matches the target address.
            This will match if a block has not been moved or no
            instructions have been inserted and the branch has a mapping
            to the first instruction. */
    }
}



/*  Goes through the symbol tables and adjust the entries
    for function addresses. It does this by checking the address
    for a function. If the function address has been changed
    the new address is set in the table. */
void binaryChanger::correctSymbolTableFunctionEntries() {
    /*  Get the sgasmelfsymbolsection. */
    std::vector<SgAsmElfSymbolSection*> elfSymbolSections =
        SageInterface::querySubTree<SgAsmElfSymbolSection>(changerProjectPtr);

    /* Debug printout. */
    if (true) {
        std::cout << "symbolsections found: " << elfSymbolSections.size() << std::endl;
    }

    /*  Get the list of symbols object from the section. */
    //TODO assuming there are several sections, iterate over them?
    SgAsmElfSymbolList* elfSymbolListObject = elfSymbolSections.front()->get_symbols();

    /*  Get the vector of sgasmelfsymbols. */
    SgAsmElfSymbolPtrList& elfSymbolList = elfSymbolListObject->get_symbols();

    /*  Check symbols if they are for a function. */
    for(SgAsmElfSymbolPtrList::iterator symbolIter = elfSymbolList.begin();
        symbolIter != elfSymbolList.end(); ++symbolIter) {
        /*  Get the SgAsmSymbol. */
        SgAsmElfSymbol* elfSym = (*symbolIter);
        /*  Check if the symbol is related to a function. */
        if (SgAsmElfSymbol::STT_FUNC == elfSym->get_elf_type()) {
            /*  Test setting the value. */
            if (1 == oldToNewAddrMap.count(elfSym->get_value())) {
                /*  printout for debugging purposes. */
                if (true) {
                    /*  Get the string object. */
                    SgAsmGenericString* nameObject = elfSym->get_name();
                    /*  Extract the string. */
                    std::cout << "func name: " << nameObject->get_string() << std::endl;
                    /*  Get the value that is related to the symbol.
                        Since it is a function this should be the address. */
                    std::cout << "value: " << std::hex << elfSym->get_value() << std::endl;
                }
                /*  Get the new address for the function. */
                rose_addr_t newSymAddr = oldToNewAddrMap.find(elfSym->get_value())->second;
                /*  Assign the new address to the symbol entry. */
                elfSym->set_value(newSymAddr);
                /*  Debug print for the new value. */
                if (true) {
                    std::cout << "new value: " << newSymAddr << std::endl;
                }
            }
        }
    }
}



/*  Function that adjusts the physical offsets in the binary.
    This needs to be done if code has been inserted somewhere
    then following sections need to be adjusted. */
void binaryChanger::fixSectionOffsets() {
    /*  Make a copy of all the elf sections. */
    std::vector<SgAsmElfSection*> physicalElfSections(elfSections);
}

