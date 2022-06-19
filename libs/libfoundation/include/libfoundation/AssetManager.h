// Begining of possible AssetManager to locate/read/write to files and folders in the app directory
namespace LFoundation {

class AssetManager {
public:
    AssetManager(std::string name) {
        app_root = "/Applications/";
        app_root += name + ".app/Content/";
    }

    ~AssetManager() = default;

    std::string find(std::string path) {
        return app_root + path;
    }
private:
    std::string app_root;
};

}