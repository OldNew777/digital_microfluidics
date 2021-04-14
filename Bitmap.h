#pragma once
class Bitmap
{
private:
	int _size;
	char * _data;

public:
	Bitmap(int n = 8);
	~Bitmap();

    void expand(int k);
    void set(int k);
    void clear(int k);
    bool test(int k);
    void clearAll();
};

