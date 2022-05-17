#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include <random>

typedef std::pair<long, long> attrs;
typedef std::map <std::string, attrs> storage;

struct request
        : public std::basic_string<char> {
    std::string name;
    long size = 0;
    long time = 0;

public:
    request() {name = "";};

    request(std::vector <std::string> req)
    {
        if (req.size() >= 3)
        {
            name = req[0];
            size = stoi(req[1]);
            time = stoi(req[2]);
        }
    }
};


class cache
{
    unsigned long size = 0;
    long occupied = 0;
    long all_bytes = 0, hit_bytes = 0;
    long all_objects = 0, hit_objects = 0;

protected:
    storage cach;

public:
    void setSize(unsigned long siz) { size = siz; }
    long getSize() {return size;}
    bool isOccupied(long reqsize) { return (occupied + reqsize) > size;}
    request remove_req();
    virtual void add_req(request req);
    request what_req();
    virtual bool find_req(request req);
    storage* get_cache() {return &cach;};
    float getBHR() {
        return float(hit_bytes)/all_bytes;};
    float getOHR() {
        return float(hit_objects)/all_objects;};

    long getOccupied() {return occupied;};
    long getAll() {return all_bytes;};
    long getHit() {return hit_bytes;};

    void add_occupied(long num) {occupied += num;};
    void add_all(long num) {all_bytes += num; all_objects++;};
    void add_hit(long num) {hit_bytes += num; hit_objects++;};

};

request cache::what_req()
{
    request res;
    std::string item = "";
    long time_min = INT64_MAX, max_size = 0;
    //std::cout << "Cache is empty: " << (cach.begin() == cach.end()) << std::endl;
    //std::cout << "IS IT TRULY EMPTY?!" << (cach.empty()) << std::endl;
    //std::cout << "And occupied is " << occupied << " and also! " << this -> getOccupied() << std::endl;
    for (auto min_search = cach.begin(); min_search != cach.end(); min_search++) {
        time_min = (((*min_search).second).second < time_min) ? ((*min_search).second).second : time_min;
    }
    //std::cout << "time min is " << time_min << std::endl;
    for (auto el = cach.begin(); el != cach.end(); el++)
    {
        if (((*el).second.second == time_min) && ((*el).second.first > max_size)) {
            item = (*el).first;
            max_size = (*el).second.first;
        }
    }
    //std::cout << "max_size is " << max_size << std::endl;
    res.name = item;
    res.size = max_size;
    res.time = time_min;
    return res;
}

request cache::remove_req()
{
    request req = what_req();
    cach.erase(req.name);
    occupied -= req.size;
    return req;
}

void cache::add_req(request req)
{
    cach[req.name] = attrs(req.size, req.time);
    occupied += req.size;
}

bool cache::find_req(request req)
{
    all_bytes += req.size;
    all_objects++;
    if (cach.find(req.name) != cach.end()) {
        hit_bytes += req.size;
        hit_objects++;
        cach[req.name].second = req.time;
        return true;
    }
    return false;
}

class LRU: public cache
{};

class LFU: public cache
{
public:

    virtual void add_req(request req);
    virtual bool find_req(request req);
};

void LFU::add_req(request req)
{
    cach[req.name] = attrs(req.size, 1);
    add_occupied(req.size);
}

bool LFU::find_req(request req)
{
    add_all(req.size);
    if (cach.find(req.name) != cach.end()) {
        add_hit(req.size);
        cach[req.name].second += 1;
        return true;
    }
    return false;
}



class TwoCache
{
    LRU head;
    LRU ghost_head;
    LFU tail;
    LRU ghost_tail;
    unsigned long size = 0;
    double part = 1.0;
    long all_bytes = 0, hit_bytes = 0;
    long all_objects = 0, hit_objects = 0;

public:
    void setSize(long siz) {size = siz;}
    void setPart(double p) {part = (p <= 1) ? p : 1/p;}
    void ghostSizes() {
        ghost_head.setSize(head.getSize()/2);
        ghost_tail.setSize(tail.getSize()/2);
    }
    void segSize() {
        head.setSize(part*size);
        tail.setSize(size-head.getSize());
    }
    //void swap_req();
    void add_req(request req);
    bool find_req(request req);
    //void check_ghosts(request req);
    void change_borders(bool num);
    float getBHR() {
        return float(hit_bytes)/all_bytes;};
    float getOHR() {
        return float(hit_objects)/all_objects;};

};

/*void TwoCache::swap_req() {
    auto head_req = head.remove_req();
    if (head_req.size < tail.getSize()) {
        while (tail.isOccupied(head_req.size)) {
            //std::cout << "Removing from tail (swap)" << std::endl;
            auto delete_req = tail.remove_req();
            if ((delete_req.size < ghost_tail.getSize())) {
                while (ghost_tail.isOccupied(delete_req.size)) {
                    auto del_req = ghost_tail.remove_req();
                    if (del_req.size == 0)
                        break;
                }
                ghost_tail.add_req(delete_req);
            }
        }
        tail.add_req(head_req);
    }
    if (head_req.size < ghost_head.getSize()) {
        while (ghost_head.isOccupied(head_req.size)) {
            auto head_del = ghost_head.remove_req();
            if (head_del.size == 0)
                break;
        }
    ghost_head.add_req(head_req);
    }
}*/

bool TwoCache::find_req(request req)
{
    all_bytes += req.size;
    all_objects++;
    if (!(tail.find_req(req))) {
        if (ghost_head.find_req(req)) {
            change_borders(true);
        }
        if (!(head.find_req(req))) {
            if (ghost_tail.find_req(req))
            {
                change_borders(false);
            }
                add_req(req);
                return false;
        }
    }
    hit_bytes += req.size;
    hit_objects++;
    return true;
}

void TwoCache::change_borders(bool num)
{
    if (num == true)
    {
        auto tail_del = tail.remove_req();
        auto size_change = tail_del.size;
        double part = (double(head.getSize()+size_change))/(size);
        setPart(part);
        segSize();
        head.add_req(tail_del);
    }
    else if (num == false)
    {
        auto head_del = head.remove_req();
        auto size_change = head_del.size;
        double part = (double(head.getSize()-size_change))/(size);
        setPart(part);
        segSize();
        tail.add_req(head_del);
    }
}

void TwoCache::add_req(request req)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::discrete_distribution<> d({part, 1-part});
    auto part_num = d(gen);
    if ((part_num == 0) && (!(head.isOccupied(req.size))))
    {
        head.add_req(req);
    }
    else if ((part_num == 1) && (!(tail.isOccupied(req.size))))
    {
        tail.add_req(req);
    }
    else if (!(head.isOccupied(req.size)))
    {
        head.add_req(req);
    }
    else if (!(tail.isOccupied(req.size)))
    {
        tail.add_req(req);
    }
    else
    {
        if ((part_num == 0) && (req.size < head.getSize()))
        {
            while (head.isOccupied(req.size))
            {
                auto head_del = head.remove_req();
                if ((head_del.size < ghost_head.getSize()) && (!(ghost_head.find_req(head_del))))
                {
                    while (ghost_head.isOccupied(head_del.size))
                    {
                        auto what_is_ghost = ghost_head.remove_req();
                    }
                    ghost_head.add_req(head_del);
                }
            }
            head.add_req(req);
        }
        else if ((part_num == 1) && (req.size < tail.getSize()))
        {
            while (tail.isOccupied(req.size))
            {
                auto tail_del = tail.remove_req();
                if ((tail_del.size < ghost_tail.getSize()) && (!(ghost_tail.find_req(tail_del))))
                {
                    while (ghost_tail.isOccupied(tail_del.size))
                    {
                        ghost_tail.remove_req();
                    }
                    ghost_tail.add_req(tail_del);
                }
            }
            tail.add_req(req);
        }
        else if (req.size < head.getSize())
        {
            while (head.isOccupied(req.size))
            {
                auto head_del = head.remove_req();
                if ((head_del.size < ghost_head.getSize()) && (!(ghost_head.find_req(head_del))))
                {
                    while (ghost_head.isOccupied(head_del.size))
                    {
                        ghost_head.remove_req();
                    }
                    ghost_head.add_req(head_del);
                }
            }
            head.add_req(req);
        }
        else if (req.size < req.size < tail.getSize())
        {
            while (tail.isOccupied(req.size))
            {
                auto tail_del = tail.remove_req();
                if ((tail_del.size < ghost_tail.getSize()) && (!(ghost_tail.find_req(tail_del))))
                {
                    while (ghost_tail.isOccupied(tail_del.size))
                    {
                        ghost_tail.remove_req();
                    }
                    ghost_tail.add_req(tail_del);
                }
            }
            tail.add_req(req);
        }
    }

}

/*void TwoCache::check_ghosts(request req)
{
    if (ghost_tail.find_req(req))
    {
        auto to_head_ghost = head.remove_req();
        auto size_change = to_head_ghost.size;
        if (size_change < ghost_head.getSize()) {
            while (ghost_head.isOccupied(size_change)) {
                ghost_head.remove_req();
            }
            ghost_head.add_req(to_head_ghost);
        }
        double part = (double(head.getSize()-size_change))/(size);
        setPart(part);
        segSize();
    }
    else if (ghost_head.find_req(req))
    {
        auto to_tail_ghost = tail.remove_req();
        auto size_change = to_tail_ghost.size;
        if (size_change < ghost_tail.getSize()) {
            while (ghost_tail.isOccupied(size_change)) {
                ghost_tail.remove_req();
            }
            ghost_tail.add_req(to_tail_ghost);
        }
        double part = 1.0 - (double(tail.getSize()-size_change))/(size);
        setPart(part);
        segSize();
    }
}*/




int main(int argc, char** argv) {
    std::string path = (argc > 1) ? argv[1] : "big_compare.txt", buffer;
    int divisor = (argc > 2) ? atoi(argv[2]) : 10000;
    float p = (argc > 3) ? atof(argv[3]) : 0.5;
    std::ifstream requests(path);
    std::vector <std::string> split_request(3);
    TwoCache LFRU;
    std::cout.precision(8);
    if (argc > 4)
        LFRU.setSize(atol(argv[4]));
    else
        LFRU.setSize(5ULL*1024*1024*1024);
    long count = 0;
    LFRU.setPart(p);
    LFRU.segSize();
    LFRU.ghostSizes();
    while (std::getline(requests, buffer)) {
        if (!(buffer.empty())) {
            for (int i = 0; i < 3; i++) {
                split_request[i] = buffer.substr(0, (buffer.find(',')));
                buffer = buffer.substr(buffer.find(',') + 1);
            }
            LFRU.find_req(request(split_request));
            if ((++count)%divisor == 0)
                std::cout << std::fixed << count << " : " << LFRU.getBHR() << " : " << LFRU.getOHR() << std::endl;
        }
    }
    std::cout << std::fixed << count << " : " << LFRU.getBHR() << " : " << LFRU.getOHR() << std::endl;
    return 0;
}

