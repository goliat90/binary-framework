#ifndef USERTRANSFORMER_H
#define USERTRANSFORMER_H

/*
This class is used by the user to define the behavior of transformations.
It is passed to the framework which calls it during traversal of the binary.
*/
class userTransformer {
    public:
        // Constructor
        userTransformer();
        // function that framework calls when traversing.
        // might change this to a function based transform instead of giving a whole block.
        void blockTransform();

    private:

};

#endif
