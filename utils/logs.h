#pragma once
#define LOGINFO std::cout << std::endl \
                          << "[INFO]: " << __FILE__ << ":" << __LINE__ << " (" << __func__ << "):  \t"

template <typename T>
std::string array_str(T *e, int t)
{
    std::string ret = "[";
    for (int i = 0; i < t - 1; ++i)
    {
        ret += std::to_string(e[i]) + ", ";
    }
    ret += std::to_string(e[t - 1]) + "]";
    return ret;
}

template <typename T>
std::string array_str(T e)
{
    int t = e.size();
    std::string ret = "[";
    for (auto it = e.begin(); it != e.end(); ++it)
    {
        ret += std::to_string(*it) + ", ";
    }
    ret += "]";
    return ret;
}