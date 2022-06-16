//#include <iostream>
//#include <vector>
//#include <unordered_map>

/*
#define memSize 1000
#define BYTE_SZ 8
#define MAX_MEM_MOD 32
#define EXIT 0
#define ERR 1

char setByte(bool x1, bool x2, bool x3, bool x4, bool x5, bool x6, bool x7, bool x8)
{
    return (x1) + (x2 << 1) + (x3 << 2) + (x4 << 3) + (x5 << 4) + (x6 << 5) + (x7 << 6) + (x8 << 7);
}

#define BOOLEAN_DEFAULT setByte(0, 0,0,1, 0,0,0,0)

char NULLHEAD = setByte(0, 1,1,1, 0,0,0,0);
char NULLPTR = -1;

/*
    head = 1 byte
     0  - to be collected
     1-3- type (8 posible types)
     4-7- size (min = 1, max = 256)

    types:
    0: Pointer = 1 byte (65536 addr)
       - Void: 0111 size 000...
    1: Bool = head + 1 byte
    2: Char = head + 1-256 bytes
    3: Int = head + 2-64 bytes
       - Twos compement
       - Operators: + - * % / <=>
    4: Str = head + 1-256 bytes + ptr

/*
#define ERROR 0
#define WARNING 1
#define MESSAGE 2

const std::vector<std::string> SEVARITIES = { "ERROR", "WARNING", "MESSAGE" };

struct err {
    int sev, type;
    std::string msg;

    friend std::ostream& operator<< (std::ostream& _stream, err& _rhs)
    {
        _stream << SEVARITIES[_rhs.sev] << " #" << _rhs.type << ": " << _rhs.msg;
        return _stream;
    }
};


struct memBlock;
struct ptr;
struct boolean;
struct integer;
struct character;
struct string;

int memset(bool _set, int _from = 0, int _to = memSize);
ptr malloc(int _size);

auto set = [](char& _byte, unsigned int _ind, bool _val = 1) { _val ? _byte |= (1 << _ind) : _byte &= (~(1 << _ind)); };
auto tog = [](char& _byte, unsigned int _ind) { _byte ^= (1 << _ind); };
auto get = [](const char& _byte, unsigned int _ind) { return _byte & (1 << _ind); };

struct memBlock {
    char start;
    char size;
    //memBlock *next, *prev;

    memBlock(char _start, char _size) : start(_start), size(_size) {}

    memBlock split(int _size)
    {
        size -= _size;
        return memBlock(start + _size, _size);
    }
    char end() { return start + size; }
};

/*
class memList {
    int count;
    std::vector<memBlock*> list;

    memBlock* head;
    memBlock* tail;

public:
    
    memList() { count = 0;  }

    // Returns the closest node to _pos, always to the right (less than) _pos
    memBlock* findRightNeighbor(char _pos)
    {
        memBlock* guess = list[(int) ((count - 1) * (1.0f * head->start / _pos))];
        bool direction = guess->start > _pos;

        while (guess->start > _pos == direction)
            direction ? guess = guess->prev : guess = guess->next;

        return direction ? guess : guess->prev;
    }

    memBlock* binarySearchRightNeighbor(char _pos)
    {
        char inc = count - 1;
        char guess = (count - 1) / 2;

        while (count > 1)
        {
            if (guess == 0 || guess == count - 1 || list[guess]->start > _pos && list[guess + 1]->start < _pos)
                return list[guess];
            else if (list[guess]->start < _pos)
                guess += (count /= 2);
            else
                guess -= (count /= 2);
        }

        return nullptr;
    }

    void insert(memBlock* _ins)
    {
        if (memBlock* rightNeighbor = binarySearchRightNeighbor(_ins->start))
        {
            rightNeighbor->next->prev = _ins;
            _ins->next = rightNeighbor->next;
            _ins->prev = rightNeighbor;
            rightNeighbor->next = _ins;

            count++;
        }
        else
            return;
    }
};


char mem[memSize];
std::vector<memBlock> free;
std::vector<memBlock> allocated;

int setByteRange(char& _byte, bool _set, int _from = 0, int _to = BYTE_SZ)
{
    if (_from >= _to || _to >= BYTE_SZ)
        return ERR;

    for (unsigned int i = _from; i < _to; i++)
        set(_byte, i, _set);

    return EXIT;
}

int copyByte(char& _curr, const char& _copy)
{
    for (unsigned int i = 0; i < BYTE_SZ; i++)
        set(_curr, i, get(_copy, i));
}

int copyByteRange(char& _curr, int _at, const char& _copy, int _from = 0, int _to = BYTE_SZ)
{
    if (_from >= _to || _to >= BYTE_SZ || _at + _from - _to >= BYTE_SZ)
        return ERR;

    for (unsigned int i = _from; i < _to; i++)
        set(_curr, i + _at, get(_copy, i));

    return EXIT;
}

int   memset(bool _set, int _from = 0, int _to = memSize)
{
    if (_from >= _to)
        return ERR;

    for (int i = _from; i < _to; i++)
        set(mem[i / BYTE_SZ], i % BYTE_SZ, _set);

    return EXIT;
}

char& memget(ptr& _at)
{
    if (_at.val >= memSize)
        return NULLPTR;

    return mem[_at.val];
}

char& memget(char& _at)
{
    if (_at >= memSize)
        return NULLPTR;

    return mem[_at];
}

ptr malloc(int _size)
{
    for(memBlock& b : free)
        if (b.size < _size)
        {
            ptr newAlloc = ptr(b.split(_size));
        }
}

int memfree(ptr& _at)
{
    for (memBlock& block : free)
    {
        if (block.end() + 1 == _at.val)
            ;
    }
    // Search free blocks to try and merge neighboring free blocks
    // Else, create a new block and append it to free
    // Change type to null
}

struct ptr
{
    char& val;

    ptr() : val(NULLPTR) {}

    // Blank pointer, just has null pointer type (111) and it's size
    ptr(memBlock block) : val(NULLHEAD)
    {
        val = memget(block.start);
        copyByteRange(val, 4, block.size, 0, 4);
    }

    //char size() { return }

    char type() {}

    ptr& operator= (const ptr& rhs) { val = rhs.val; return *this; }

    operator char() { return val; }
};

struct boolean {
    char& val;
    ptr loc;

    // Create a new boolean
    boolean(bool b) : val(NULLPTR)
    {
        loc = malloc(1);
        val = memget(loc);
        copyByte(val, BOOLEAN_DEFAULT);
        setByteRange(val, b, 4);
    }

    // Get value from pointer
    boolean(ptr& _from) : val(NULLPTR)
    {
        val = memget(_from);
    }

    ~boolean() { memfree(loc); }

    boolean& operator= (const boolean& rhs) { val = rhs.val; return *this; }
    bool operator== (const boolean& rhs) { return get(rhs.val, 4) == get(val, 4); }

    operator char() { return val; }
    operator bool() { return get(val, 4); }
};

struct integer {
    char& head;
    ptr loc;
};

int main()
{
	memset(false);


}
*/

int main() { return 0; }