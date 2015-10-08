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
    postBlockChanges();

    /*  Reallocate segments. */
    reallocateSegments();

    /*  Calculate the space between segments. */
    findFreeVirtualSpace();
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
    std::sort(basicBlockVector.begin(), basicBlockVector.end(), blockOrder);
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
                    lowerVirtualAddressLimit = (*elfIter)->get_mapped_preferred_va();
                }
                /*  Check if the header if Read and Writable. */
                if (true == (*elfIter)->get_mapped_rperm() && true == (*elfIter)->get_mapped_wperm()) {
                    /*  The section is relevant, save it. */
                    sectionVector.push_back(*elfIter);
                    /*  Save the base address for this section. It will be used
                        as a upper limit. This is assuming that this section is
                        after the code segment and there is only one data segment. */
                    upperVirtualAddressLimit = (*elfIter)->get_mapped_preferred_va();
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
            std::cout << "Address (mapped_preferred_va): " << std::hex << (*elfIter)->get_mapped_preferred_va() << std::endl;
            /* size of section. */
            std::cout << "Size (mapped): " << std::hex << (*elfIter)->get_mapped_size() << std::endl;
            std::cout << "Size (file)  : " << std::hex << (*elfIter)->get_size() << std::endl;
            /*  offsets. */
            std::cout << "Offset(file) : " << std::hex << (*elfIter)->get_offset() << std::endl;

            std::cout << std::endl;
        }
    }

    /* Go through the elf sections and add any section that is whithin the address range. */
    for(asmElfVector::iterator elfIter = elfSections.begin();
        elfIter != elfSections.end(); ++elfIter) {
        /*  For each section check its address (virtual). */
        rose_addr_t elfVa = (*elfIter)->get_mapped_preferred_va();
        /*  See if it should be added or not. */
        if (lowerVirtualAddressLimit <= elfVa && elfVa <= upperVirtualAddressLimit) {
            /* The section is within the bound, check so it is not the
                section itself by searching the sectionVector. */
            if (sectionVector.end() == std::find(sectionVector.begin(), 
                    sectionVector.end(), (*elfIter))) {
                /*  The elf section is not a section itself. */
                segmentVector.push_back(*elfIter);
            }
        }
    }

    /*  Sort the section according to addresses. */
    std::sort(sectionVector.begin(), sectionVector.end(), elfSectionOrder);

    /*  Sort the segments according to their virtual addresses. */
    std::sort(segmentVector.begin(), segmentVector.end(), elfSectionOrder);

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
            std::cout << "Address (mapped_preferred_va): " << std::hex << (*elfIter)->get_mapped_preferred_va() << std::endl;
            /* size of section. */
            std::cout << "Size (mapped): " << std::hex << (*elfIter)->get_mapped_size() << std::endl;
            std::cout << "Size (file)  : " << std::hex << (*elfIter)->get_size() << std::endl;
            /*  offsets. */
            std::cout << "Offset(file) : " << std::hex << (*elfIter)->get_offset() << std::endl;

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
            std::cout << "Address (mapped_preferred_va): " << std::hex << (*elfIter)->get_mapped_preferred_va() << std::endl;
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
void binaryChanger::postBlockChanges() {
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
            blockNewSize.insert(std::pair<SgAsmBlock*, int>((*blockIter), newSize));
        }
    }

    //TODO perhaps go through here and check the sizes of segments
    //TODO probbably need to take gaps between the bloocks in into
    //TODO consideration when calculating the size of a segment.
}


/*  Start moving segments that can not grow in its current place. */
void binaryChanger::reallocateSegments() {

}

/*  Go through the address space and find the open spaces. */
//TODO consider calling this function when i have moved a segment so
//TODO can continue placement
void binaryChanger::findFreeVirtualSpace() {
    
}





