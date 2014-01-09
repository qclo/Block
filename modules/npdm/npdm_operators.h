#ifndef NPDM_OPERATORS_HEADER
#define NPDM_OPERATORS_HEADER
#include "BaseOperator.h"

namespace SpinAdapted{

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// 3PDM operators
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

class DesCre: public SpinAdapted::SparseMatrix
{
  public:
    DesCre() { orbs.resize(2); fermion = false; build_pattern = "(DC)";} // default build_pattern
    void build_in_csf_space(const SpinBlock& b);
    void build(const SpinBlock& b) ;
    void build_from_disk(SpinBlock& b, std::ifstream& sysfs, std::ifstream& dotfs) { assert(false); }
    boost::shared_ptr<SparseMatrix> getworkingrepresentation(const SpinBlock* block);
    double redMatrixElement(Csf c1, vector<Csf>& ladder, const SpinBlock* b);
};

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

class CreCreDes: public SpinAdapted::SparseMatrix
{
  public:
    CreCreDes() { orbs.resize(3); fermion = true; build_pattern = "((CC)D)";} // default build_pattern
    void build_in_csf_space(const SpinBlock& b);
    void build(const SpinBlock& b) ;
    void build_from_disk(SpinBlock& b, std::ifstream& sysfs, std::ifstream& dotfs);
    boost::shared_ptr<SparseMatrix> getworkingrepresentation(const SpinBlock* block);
    double redMatrixElement(Csf c1, vector<Csf>& ladder, const SpinBlock* b);
};

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

class CreDesDes: public SpinAdapted::SparseMatrix
{
  public:
    CreDesDes() { orbs.resize(3); fermion = true; build_pattern = "((CD)D)";} // default build_pattern
    void build_in_csf_space(const SpinBlock& b) {assert(false);}
    void build(const SpinBlock& b) ;
    void build_from_disk(SpinBlock& b, std::ifstream& sysfs, std::ifstream& dotfs);
    boost::shared_ptr<SparseMatrix> getworkingrepresentation(const SpinBlock* block);
    double redMatrixElement(Csf c1, vector<Csf>& ladder, const SpinBlock* b);
};

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

class CreDesCre: public SpinAdapted::SparseMatrix
{
  public:
    CreDesCre() { orbs.resize(3); fermion = true; build_pattern = "((CD)C)";} // default build_pattern
    void build_in_csf_space(const SpinBlock& b);
    void build(const SpinBlock& b) ;
    void build_from_disk(SpinBlock& b, std::ifstream& sysfs, std::ifstream& dotfs);
    boost::shared_ptr<SparseMatrix> getworkingrepresentation(const SpinBlock* block);
    double redMatrixElement(Csf c1, vector<Csf>& ladder, const SpinBlock* b);
};

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

class CreCreCre: public SpinAdapted::SparseMatrix
{
  public:
    CreCreCre() { orbs.resize(3); fermion = true; build_pattern = "((CC)C)";} // default build_pattern
    void build(const SpinBlock& b) ;
    void build_from_disk(SpinBlock& b, std::ifstream& sysfs, std::ifstream& dotfs);
    boost::shared_ptr<SparseMatrix> getworkingrepresentation(const SpinBlock* block);
    double redMatrixElement(Csf c1, vector<Csf>& ladder, const SpinBlock* b);
};

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// 4PDM operators
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

class DesCreDes: public SpinAdapted::SparseMatrix
{
  public:
    DesCreDes() { orbs.resize(3); fermion = true; build_pattern = "((DC)D)";} // default build_pattern
    void build(const SpinBlock& b) ;
    void build_from_disk(SpinBlock& b, std::ifstream& sysfs, std::ifstream& dotfs);
    boost::shared_ptr<SparseMatrix> getworkingrepresentation(const SpinBlock* block);
    double redMatrixElement(Csf c1, vector<Csf>& ladder, const SpinBlock* b);
};

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

class DesDesCre: public SpinAdapted::SparseMatrix
{
  public:
    DesDesCre() { orbs.resize(3); fermion = true; build_pattern = "((DD)C)";} // default build_pattern
    void build(const SpinBlock& b) ;
    void build_from_disk(SpinBlock& b, std::ifstream& sysfs, std::ifstream& dotfs);
    boost::shared_ptr<SparseMatrix> getworkingrepresentation(const SpinBlock* block);
    double redMatrixElement(Csf c1, vector<Csf>& ladder, const SpinBlock* b);
};

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

class CreCreDesDes: public SpinAdapted::SparseMatrix
{
  public:
    CreCreDesDes() { orbs.resize(4); fermion = true; build_pattern = "((CC)(DD))";} // default build_pattern
    void build(const SpinBlock& b) ;
    void build_from_disk(SpinBlock& b, std::ifstream& sysfs, std::ifstream& dotfs) { assert(false); }
    boost::shared_ptr<SparseMatrix> getworkingrepresentation(const SpinBlock* block);
    double redMatrixElement(Csf c1, vector<Csf>& ladder, const SpinBlock* b);
};

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

}

#endif
