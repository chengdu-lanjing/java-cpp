#include <iostream>
#include <database/Sqlite.h>
#include <log/Logger.h>

using namespace std;
using namespace com_lanjing_cpp_common;
using namespace com_lanjing_cpp_common_database;

namespace demo_database {

    class Product : extends Object {
    public:
        Nullable<int> getId() const {
            return this->id;
        }
        const string &getName() const {
            return this->name;
        }
        float getPrice() const {
            return this->price;
        }
        virtual string toString() const override {
            ostringstream oss;
            oss
                    << "{ id: " << this->id
                    << ", name: \"" << this->name
                    << "\", price: " << this->price << " }";
            return oss.str();
        }
    public:
        class Builder : extends Object {
        public:
            Ref<Builder> setId(Nullable<int> id) {
                this->getTarget()->id = id;
                return this;
            }
            Ref<Builder> setName(const string &name) {
                this->getTarget()->name = name;
                return this;
            }
            Ref<Builder> setPrice(float price) {
                this->getTarget()->price = price;
                return this;
            }
            Ref<Builder> cloneFrom(Ref<Product> product) {
                Ref<Product> tgt = this->getTarget();
                tgt->id = product->id;
                tgt->name = product->name;
                tgt->price = product->price;
                return this;
            }
            Ref<Product> build() {
                Ref<Product> tgt = this->getTarget();
                this->target = nullptr;
                return tgt;
            }
        private:
            Builder() {}
            Ref<Product> getTarget() {
                Ref<Product> tgt = this->target;
                if (tgt == nullptr) {
                        tgt = new_internal(Product); //Product::Product() is not public
                        this->target = tgt;
                }
                return tgt;
            }
        private:
            Ref<Product> target;
            friend class Product;
        };
        static Ref<Builder> builder() {
            static Ref<ThreadLocal<Builder>> bldrLocal = new_<ThreadLocal<Builder>>();
            Ref<Builder> bldr = bldrLocal->get();
            if (bldr == nullptr) {
                bldr = new_internal(Builder); //Builder::Builder() is not public
                bldrLocal->set(bldr);
            }
            return bldr;
        }
    private:
        Product() {}
    private:
        Nullable<int> id;
        string name;
        float price;
        friend class Builder;
    };

    class ProductRepository : extends Object {
        declare_logger(ProductRepository)
    public:
        ProductRepository(Ref<Connection> con) : con(con) {}
        Ref<Product> findByName(const string &name) {
            Ref<Statement> stmt = this->con
                ->preparedStatement("select id, name, price from product where name = :name")
                ->set(":name", name);
            Ref<ResultSet> rs = stmt->executeQuery();
            if (rs->next()) {
                return mapProduct(stmt->executeQuery());
            }
            return nullptr;
        }
        vector<Ref<Product>> findAll() {
            Ref<Statement> stmt = this->con->preparedStatement(
                "select id, name, price from product order by id asc"
            );
            return mapProducts(stmt->executeQuery());
        }
        vector<Ref<Product>> findLikeName(const string &namePattern) {
            string lowerPattern = namePattern;
            for (auto &ch : lowerPattern) {
                if (ch >= 'A' && ch < 'Z') {
                    ch += 'a' - 'A';
                }
            }
            Ref<Statement> stmt = this->con
                ->preparedStatement(
                        "select id, name, price from product \
                        where lower(name) like :namePattern \
                        order by name asc"
                )
                ->set(":namePattern", lowerPattern);
            return mapProducts(stmt->executeQuery());
        }
        vector<Ref<Product>> findByPriceRange(float minPrice, float maxPrice) {
            Ref<Statement> stmt = this->con
                ->preparedStatement(
                        "select id, name, price from product \
                        where price between :minPrice and :maxPrice \
                        order by price asc"
                )
                ->set(":minPrice", minPrice)
                ->set(":maxPrice", maxPrice);
            return mapProducts(stmt->executeQuery());
        }
        void merge(Ref<Product> product) {
            if (product == nullptr) {
                throw_new(IllegalArgumentException, "product cannot be null");
            }
            if (product->getId() == nullptr) {
                this->con
                    ->preparedStatement("insert into product(name, price) values(:name, :price)")
                    ->set(":name", product->getName())
                    ->set(":price", product->getPrice())
                    ->executeUpdate();
            } else {
                this->con
                    ->preparedStatement("update product set name = :name, price = :price where id = :id")
                    ->set(":id", product->getId())
                    ->set(":name", product->getName())
                    ->set(":price", product->getPrice())
                    ->executeUpdate();
            }
        }
        int clear() {
            return this->con->preparedStatement("delete from product")->executeUpdate();
        }
    protected:
        virtual void initialize() override {
            this->createSchemaIfNecessary();
        }
    private:
        void createSchemaIfNecessary() {
            Ref<Statement> stmt = this->con
                ->preparedStatement("select count(*) from sqlite_master where type = :type and name = :name")
                ->set(1, "table")
                ->set(2, "product");
            Ref<ResultSet> rs = stmt->executeQuery();
            if (rs->next() && rs->getInt(1) > 0) {
                logger().info("The table '{}' already exists, skip schema creating step", "product");
                return;
            }
            logger().info("Create table '{}'", "product");
            stmt = con->preparedStatement(
                "create table product(\
                    id integer primary key autoincrement not null, \
                    name text not null, \
                    price decimal(10, 2) not null, \
                    constraint unq_product_name unique(name) \
                )"
            );
            stmt->executeUpdate();
        }
        static vector<Ref<Product>> mapProducts(Ref<ResultSet> rs) {
            vector<Ref<Product>> products;
            while (rs->next()) {
                products.push_back(mapProduct(rs));
            }
            return products;
        }
        static Ref<Product> mapProduct(Ref<ResultSet> rs) {
            return Product::builder()
                ->setId(rs->getInt(1))
                ->setName(rs->getString(2))
                ->setPrice(rs->getDouble(3))
                ->build();
        }
    private:
        Ref<Connection> con;
    };
}

using namespace demo_database;

int main(int argc, char *argv[]) {

    /*
     * It's very important to register Sqlite Driver,
     * the lifecyle of the driver object must cover all the database operations.
     */
    Ref<Driver> driver = new_<Sqlite3Driver>();

    Ref<Connection> con = DriverManager::getConnection("cdbc:sqlite:/tmp/sqlite.db");
    Ref<ProductRepository> productRepository = new_<ProductRepository>(con);

    cout << "-----------------------------------" << endl;
    cout << "Clear products" << endl;
    int affectedRowCount = productRepository->clear();
    cout << "Cleared, affected row count is " << affectedRowCount << endl;

    cout << "-----------------------------------" << endl;
    cout << "Insert rows" << endl;
    productRepository->merge(Product::builder()->setName("Egg")->setPrice(3.6F)->build());
    productRepository->merge(Product::builder()->setName("Pork")->setPrice(16.2F)->build());
    productRepository->merge(Product::builder()->setName("Cake")->setPrice(14.5F)->build());
    productRepository->merge(Product::builder()->setName("Beef")->setPrice(25.3F)->build());

    cout << "-----------------------------------" << endl;
    cout << "Query all the products" << endl;
    for (Ref<Product> product : productRepository->findAll()) {
        cout << product << endl;
    }

    cout << "-----------------------------------" << endl;
    cout << "Query products like '%e%'" << endl;
    for (Ref<Product> product : productRepository->findLikeName("%e%")) {
        cout << product << endl;
    }

    cout << "-----------------------------------" << endl;
    cout << "Query products by price range [10, 15]" << endl;
    for (Ref<Product> product : productRepository->findByPriceRange(10, 15)) {
        cout << product << endl;
    }

    cout << "-----------------------------------" << endl;
    cout << "Modify the price of 'Pork' to be 14.9" << endl;
    Ref<Product> pork = productRepository->findByName("Pork");
    if (pork != nullptr) {
        productRepository->merge(Product::builder()->cloneFrom(pork)->setPrice(14.9)->build());
    } else {
        cout << "There is no product whose name is 'Pork'" << endl;
    }

    cout << "-----------------------------------" << endl;
    cout << "Query products by price range [10, 15]" << endl;
    for (Ref<Product> product : productRepository->findByPriceRange(10, 15)) {
        cout << product << endl;
    }

    return 0;
}
