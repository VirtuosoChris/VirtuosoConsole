#ifndef BASELOG_H_INCLUDED
#define BASELOG_H_INCLUDED

namespace Virtuoso{
class BaseLog{

    public:

    virtual std::ostream& status()=0;
    virtual std::ostream& error()=0;
    virtual std::ostream& echo()=0;
    virtual std::ostream& warning()=0;


    inline std::ostream& operator()(){return status();}

};

}

#endif // BASELOG_H_INCLUDED
