#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include <numeric>
#include <algorithm>

typedef std::pair<long, long> attrs;
typedef std::map <std::string, attrs> storage;
typedef std::map <std::string, bool> chances;

struct request
        : public std::basic_string<char> {
    std::string name;
    long size = 0;
    long times = 0;

public:
    request(std::vector <std::string> req)
    {
        if (req.size() >= 2)
        {
            name = req[0];
            size = stoi(req[1]);
        }
    }
};


class cache
{
    storage cach;
    std::vector<std::string> all_names;
    std::vector<long> all_times;
    std::vector<bool> chances;
    unsigned long size = 0;
    long occupied = 0;
    long all_bytes = 0, hit_bytes = 0;
    long all_objects = 0, hit_objects = 0;
    unsigned long clock_counter = 0;

public:
    void setSize(unsigned long siz) { size = siz; }
    bool isOccupied(long reqsize) { return (occupied + reqsize) > size;}
    void remove_req();
    void add_req(request req);
    bool find_req(request req);
    float getBHR() {
        return float(hit_bytes)/all_bytes;};
    float getOHR() {
        return float(hit_objects)/all_objects;};

};

void cache::remove_req()
{
    std::string item_to_remove = *(all_names.begin() + clock_counter);
    long counter = 0;
    bool found_bad = false;
    for (auto beg_it = chances.begin()+clock_counter; beg_it != chances.end(); beg_it++)
    {
        if (*beg_it == true)
        {
            chances[clock_counter+counter] = false;
            counter++;
        }
        else
        {
            item_to_remove = *(all_names.begin() + clock_counter + counter);
            found_bad = true;
            break;
        }
    }
    long new_counter = 0;
    if (!found_bad)
    {
        for (auto beg_it = chances.begin(); beg_it != chances.begin()+clock_counter; beg_it++)
        {
            if (*beg_it == true)
            {
                chances[new_counter] = false;
                new_counter++;
            }
            else
            {
                item_to_remove = *(all_names.begin() + new_counter);
                break;
            }    
        }
    }
    /*long times_min = INT64_MAX, max_size = 0;
    for (auto min_search = cach.begin(); min_search != cach.end(); min_search++)
        times_min = (((*min_search).second).second < times_min) ? ((*min_search).second).second : times_min;
    //std::cout << time_min << " is time_min " << std::endl;
    std::string item_to_remove;
    for (auto el = cach.begin(); el != cach.end(); el++)
    {
        if (((*el).second.second == times_min) && ((*el).second.first > max_size)) {
            item_to_remove = (*el).first;
            max_size = (*el).second.first;
        }
    }*/
    //auto old_clock_counter = clock_counter;
    //clock_counter = (clock_counter+counter)%cach.size();
    clock_counter = (found_bad) ? (clock_counter + counter) : new_counter;
    occupied -= cach[item_to_remove].first;
    //std::cout << occupied << std::endl;
    /*std::cout << cach[item_to_remove].first << std::endl;
    std::cout << counter << std::endl;
    std::cout << all_times.size() << std::endl;
    std::cout << all_names.size() << std::endl;
    std::cout << chances.size() << std::endl;*/
    all_times.erase(all_times.begin() + clock_counter);
    all_names.erase(all_names.begin() + clock_counter);
    chances.erase(chances.begin() + clock_counter);
    cach.erase(item_to_remove);
    //occupied -= max_size;
}

void cache::add_req(request req)
{
    cach[req.name] = attrs(req.size, req.times);
    
    int counter = 0;
    for (auto el = all_times.begin(); el != all_times.end(); el++)
    {
        if (*el > req.times)
                break;
        counter++;
    }
    all_times.insert(all_times.begin()+counter, req.times);
    all_names.insert(all_names.begin()+counter, req.name);
    chances.insert(chances.begin()+counter, false);
    occupied += req.size;
}

bool cache::find_req(request req)
{
    all_bytes += req.size;
    all_objects++;
    if (cach.find(req.name) != cach.cend()) {
        hit_bytes += req.size;
        hit_objects++;
        cach[req.name].second = req.times;
        long num = std::find(all_names.begin(), all_names.end(), req.name) - all_names.begin();
        chances[num] = true;
        all_times[num] = req.times;
        return true;
    }
    else
    {
        if (req.size < size)
        {
           while (isOccupied(req.size)) {
                remove_req();
           }
           add_req(req);
        }
    }
    return false;
}


int main(int argc, char** argv) {
    std::string path = (argc > 1) ? argv[1] : "big_compare.txt", buffer;
    int divisor = (argc > 2) ? atoi(argv[2]) : 10000;
    std::ifstream requests(path);
    std::vector <std::string> split_request(3);
    cache LRU;
    std::cout.precision(8);
    if (argc > 3)
        LRU.setSize(atol(argv[3]));
    else
        LRU.setSize(5ULL*1024*1024*1024);
    long count = 0;
    //int all_bytes = 0, hit_bytes = 0;
    while (std::getline(requests, buffer)) {
        if (!(buffer.empty())) {
            for (int i = 0; i < 3; i++) {
                split_request[i] = buffer.substr(0, (buffer.find(',')));
                buffer = buffer.substr(buffer.find(',') + 1);
            }
            LRU.find_req(request(split_request));
            if ((++count)%divisor == 0)
                std::cout << std::fixed << count << " : " << LRU.getBHR() << " : " << LRU.getOHR() << std::endl;
        }
    }
    std::cout << std::fixed << count << " : " << LRU.getBHR() << " : " << LRU.getOHR() << std::endl;
    return 0;
}
