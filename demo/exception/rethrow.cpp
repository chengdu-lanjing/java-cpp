#include <iostream>
#include <map>
#include <Common.h>

using namespace std;
using namespace com_lanjing_cpp_common;

namespace demo_exception {

    class Company : extends Object {
    public:
        Company(const string &name, const string &address) : name(name), address(address) {}
        const string &getName() const { return this->name; }
        const string &getAddress() const { return this->address; }
        virtual string toString() const override {
            return Object::className(this) + '@' + this->name;
        }
    private:
        string name;
        string address;
    };

    interface CompanyRepository : extends Interface {
        virtual Ref<Company> findByName(const string &name) = 0;
        virtual void save(Ref<Company> company) = 0;
    };

    class FakeCompanyRepositoryImpl : extends Object, implements CompanyRepository {
    public:
        virtual Ref<Company> findByName(const string &name) {
            auto itr = this->companyMap.find(name);
            return itr == this->companyMap.end() ? nullptr : itr->second;
        }
        virtual void save(Ref<Company> company) {
            if (this->companyMap.find(company->getName()) != this->companyMap.end()) {
                ostringstream oss;
                oss << "The company whose name is '" << company->getName() << "' already exists";
                throw_new(IllegalArgumentException, oss.str());
            }
            this->companyMap[company->getName()] = company;
        }
    private:
        map<string, Ref<Company>> companyMap;
        interface_refcount() //Implements retain&release of interfaces
    };

    class BaseDataService : extends Object {
    public:
        BaseDataService(Ref<CompanyRepository> companyRepository) : companyRepository(companyRepository) {}
        void saveCompany(Ref<Company> company) {
            cout << "Begin transaction********" << endl;
            try_ {
                this->companyRepository->save(company);
            } catch_(Exception, ex) {
                cout << "Rollback transaction********" << endl;
                throw_(ex); //Use macro 'throw_' to rethrow the original exception, NOT keyword "throw"!!!
            } end_try
            cout << "Commit transaction********" << endl;
        }
    private:
        Ref<CompanyRepository> companyRepository;
    };
}

using namespace demo_exception;

int main(int argc, char *argv[]) {
    try_ {
        Ref<BaseDataService> baseDataService = new_<BaseDataService>(new_<FakeCompanyRepositoryImpl>());
        baseDataService->saveCompany(new_<Company>("Alibaba", "HangZhou"));
        baseDataService->saveCompany(new_<Company>("Alibaba", "HnagZhou"));
    } catch_(Exception, ex) {
        ex->printStackTrace();
    } end_try
    return 0;
}
