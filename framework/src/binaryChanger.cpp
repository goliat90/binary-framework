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
    //TODO consider adding in this function finding the gaps in the address space.
    //TODO 
    preSegmentSectionCollection();

    /*  Analyze all the basic blocks and collect information about them. */
    preBlockInformationCollection();
 
}


/*  Post tranformation function. Fixes the binary so it
    is fixed. */
void binaryChanger::postTransformationWork() {
    /*  Check how the blocks have changed in size. */
    postChanges();

    /*  Reallocate segments. */
    reallocateSegments();

    /*  Calculate the space between segments. */
    //findFreeVirtualSpace();
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
        blockStartAddrMap.insert(std::pair<SgAsmBlock*, rose_addr_t>(nodeBlock, nodeBlock->get_id()));
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
        rose_addr_t elfVa = (*elfIter)->get_mapped_preferred_va();
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
            std::cout << "Size (mapped)   : " << std::hex << (*elfIter)->get_mapped_size() << std::endl;
            std::cout << "Size (file)     : " << std::hex << (*elfIter)->get_size() << std::endl;
            /*  Alignment of section in file. */
            std::cout << "alignment (file): " << std::hex << (*elfIter)->get_file_alignment() << std::endl;
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
            std::cout << "Size (mapped): " << std::hex << (*elfIter)->get_mapped_size() << std::endl;
            std::cout << "Size (file)  : " << std::hex << (*elfIter)->get_size() << std::endl;
            /*  offsets. */
            std::cout << "Offset(file) : " << std::hex << (*elfIter)->get_offset() << std::endl;

            std::cout << std::endl;
            
        }
        std::cout << "---- End Segments ----" << std::endl << std::endl;
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

    //TODO perhaps go through here and check the sizes of segments
    //TODO probbably need to take gaps between the bloocks in into
    //TODO consideration when calculating the size of a segment.

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
        rose_addr_t segAddr = (*segIter)->get_mapped_preferred_va();
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
            segmentSizeDifference.left.insert(std::pair<SgAsmElfSection*, rose_addr_t>(*segIter, segDiff));
            /*  Printout debugging information. */
            if (debugging) {
                /*  Extract the name of the segment. */
                SgAsmGenericString* elfString = (*segIter)->get_name();
                /*  Get the string name. */
                std::cout << "Name: " << elfString->get_string()
                    << " Size has changed with " << (segDiff*4) << " bytes." << std::endl;
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
        //TODO the space might have to be multiplied by 4, if it is the number of instructions.
        //TODO one instruction is 4 bytes
        rose_addr_t neededSegSpace = modifiedElfSections.back();
        SgAsmElfSection* checkedSegment = segmentSizeDifference.right.find(neededSegSpace)->second;
        rose_addr_t segSize = checkedSegment->get_mapped_size();

        if (debugging) {
            /*  Extract the name of the segment. */
            SgAsmGenericString* elfString = checkedSegment->get_name();
            std::cout << "Checking if " << elfString->get_string() 
                << " needs to be moved." << std::endl;
        }

        /*  If it cant grow in its position then move it to an address
            space where it fits. At this moment adjust the size so it is correct. */
        //TODO can i use the addressvoids map? If a segment has an entry then i know if it can grow there.
        //TODO otherwise i need to move it. 
        if (1 == addressVoids.right.count(checkedSegment)) {
            /*  The segment has space after it, check if it is enough. */
            rose_addr_t addrSpace = addressVoids.right.find(checkedSegment)->second;
            /*  Check if the available space is enough. */
            if (neededSegSpace <= addrSpace) {
                /*  The space is enough. */
                if (debugging) {
                    std::cout << "Segment can grow in place, no move needed." << std::endl;
                }
                //TODO leave segment in place and adjust the size of it.
                //TODO remove the segment from the modified list. by poping back
                //TODO perhaps have a continue here?
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
                hold the original size plus the new space. */
            if ((segSize + neededSegSpace) <= addrVoidIter->first) {
                /*  Found space after a segment that is large enough. */
                SgAsmElfSection* segment = addrVoidIter->second;
                /*  Calculate the new base address for the segment. */
                newAddress = segment->get_mapped_preferred_rva() + segment->get_mapped_size();
                /*  Set flag to true. */
                segmentMoved = true;
                /*  debugging. */
                if (debugging) {
                    SgAsmGenericString* elfString = segment->get_name();
                    std::cout << "Placing segment after segment " << elfString->get_string() << std::endl
                                << "New calculated address: " << std::hex << newAddress << std::endl;
                }
                /*  Break loop. */
                break;
            }
        }

        /*  Check if the segment was reallocated or it failed. */
        if (true == segmentMoved) {
            /*  Set the new virtual address of the segment and set the new size of it.
                This is so when the next address voids are found it will not be incorrect. */
            //TODO the address is not correct.
            checkedSegment->set_mapped_preferred_rva(newAddress);
            checkedSegment->set_mapped_size(segSize + neededSegSpace);
            if (debugging) {
                SgAsmGenericString* elfString = checkedSegment->get_name();
                std::cout << "Segment: " <<  elfString->get_string() << " moved." << std::endl
                    << "new address: " << std::hex << checkedSegment->get_mapped_preferred_va() << std::endl
                    //TODO the size might be wrong here, migth need to multiply with bytes. (*4)
                    << "new size: " << std::hex << checkedSegment->get_mapped_size() << std::endl;
            }
        } else {
            /*  Failed to move segment so throw error. */
        }

        modifiedElfSections.clear();

        /*  Remove the segment from the list of modified segments since it
            is in a acceptable place. */
    }

    //TODO possible conservative restriction, do not allow placement
    //TODO of segments on an address below the first original segment.

}

/*  Go through the address space and find the open spaces. */
//TODO consider calling this function when i have moved a segment so
//TODO can continue placement
void binaryChanger::findFreeVirtualSpace() {
    //TODO perhaps clear the container specifying the free space.
    /*  Clearing address voids. */
    addressVoids.clear();

    /*  Sort the segment so they are in order before iterating over it. */
    std::sort(segmentVector.begin(), segmentVector.end(), elfSectionSortStruct());

    for(asmElfVector::iterator segIter = segmentVector.begin();
        segIter != segmentVector.end(); ++segIter) {
        /*  Get address and size of the segment. */
        rose_addr_t segAddr = (*segIter)->get_mapped_preferred_va();
        rose_addr_t segSize = (*segIter)->get_mapped_size();
        /*  Calculate the end address of the segment. */
        rose_addr_t segEndAddr = segAddr + segSize;
        /*  Retrieve the next segments start address, or
            if it is the last segment then the next sections start address. */
        rose_addr_t nextSegAddr = 0;
        if (segmentVector.end() != (segIter+1)) {
            /*  It is not the last segment we are checking so
                there is another segment which an address can be retrieved from. */
            nextSegAddr = (*(segIter+1))->get_mapped_preferred_va();
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
            //TODO add debuging printout
            if (debugging) {
                /*  Get the string name. */
                SgAsmGenericString* elfString = (*segIter)->get_name();
                /*  Print name and space available. */
                std::cout << "Space found after segment " 
                    << elfString->get_string() << std::endl
                    << "Available space is " << std::hex << addrSpace << " bytes" << std::endl;
                    //TODO verify that it is bytes i have here.
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





