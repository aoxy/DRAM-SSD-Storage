#include <fstream>
#include <sstream>
#include <iomanip>
#include <cassert>
struct EmbFile
{
    std::fstream fs;
    size_t app_count;
    size_t version;
    std::string filepath;

    EmbFile(std::string path_, size_t ver = 0)
    {
        version = ver;
        std::stringstream ss;
        ss << std::setw(4) << std::setfill('0') << ver << ".emb";
        filepath = path_ + ss.str();
        std::remove(filepath.c_str());
        fs.open(filepath,
                std::ios::app | std::ios::in | std::ios::out | std::ios::binary);
        assert(fs.good());
        app_count = 0;
    }

    ~EmbFile()
    {
        if (fs.is_open())
        {
            fs.close();
        }
        std::remove(filepath.c_str());
    }

    void Write(const char *val, const size_t val_len)
    {
        if (fs.is_open())
        {
            fs.write(val, val_len);
        }
        else
        {
            //  LOG(INFO) << "Write Open Close";
            fs.open(filepath, std::ios::app | std::ios::in | std::ios::out |
                                  std::ios::binary);
            fs.write(val, val_len);
            fs.close();
        }
    }

    void Read(char *val, const size_t val_len, const size_t offset)
    {
        if (fs.is_open())
        {
            fs.seekg(offset, std::ios::beg);
            fs.read(val, val_len);
        }
        else
        {
            // LOG(INFO) << "Read Open Close";
            fs.open(filepath, std::ios::app | std::ios::in | std::ios::out |
                                  std::ios::binary);
            fs.seekg(offset, std::ios::beg);
            fs.read(val, val_len);
            fs.close();
        }
    }
};