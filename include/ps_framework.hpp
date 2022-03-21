
#ifndef PS_FRAMEWORK
#define PS_FRAMEWORK


// This is the base function class which is basically a linear function
class FunctionBase {
    public:
        double getRoot();
        double compute(double lambda);
        FunctionBase(double x, double y);
        FunctionBase();
        FunctionBase operator+ (const FunctionBase&);
        FunctionBase operator- (const FunctionBase&);
        bool operator== (const FunctionBase&);
        FunctionBase (const FunctionBase &f1);
        // a and b are used by the framework for comparison
        // should maybe abstracted to some other class or a part
        // of this
        double a;
        double b;
    private:
};

// Interface for the sequiential algorithm
class ISeqAlgo {
    public:
        virtual int compare(double lambda) = 0;
};

// The parametric framework class containing all relavant methods and fields
// This should maybe be in a namespace of its own
class PSFramework{
    public:
        // Compare two functions at lambda*
        // The value returned means the following
        // when comparing the functions at lambda*
        // return -1 if f1 < f2
        // return 1 if f1 > f1
        // return 0 if f1==f2
        int compare(FunctionBase f1, FunctionBase f2);
        // Set the sequintial algorithm
        void setSeqAlgo(ISeqAlgo *seqAlgo);
        // Set the problem instance 
        // void setProblemInstance(IProblemInstance *pInstance);
        PSFramework();
        // Contructor for setting the interval from the start
        PSFramework(double e, double f);
    
    private:
        // The start of the interval where we know lambda* is in, (e)
        double start;
        // The end of the interval where we know lambda* is in, (f)
        double end;
        // Sequiential algorithm used when comparing functions
        ISeqAlgo *seqAlgo;
        // internal function to check if value is inside the search interval
        bool isInInterval(double x);
};


#endif