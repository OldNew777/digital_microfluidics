#include "Bitmap.h"
#include <string.h>


Bitmap::Bitmap(int n)
{
	int bit_num = (n + 7) >> 3;
	_data = new char[bit_num];
    _size = bit_num << 3;
    memset(_data, 0, bit_num * sizeof(char));
}


Bitmap::~Bitmap()
{
	delete[] _data;
	_data = nullptr;
}

void Bitmap::expand(int k)
{
	if (k <= _size)
		return;
	char * tmp = _data;
	int _bit_num_new = (k + 7) >> 3;
	_data = new char[_bit_num_new];
    memset(_data, 0, _bit_num_new * sizeof(char));
	for (int i = 0; i < ((_size + 7) >> 3); i++)
		_data[i] = tmp[i];
	_size = k;
	delete[] tmp;
}

void Bitmap::set(int k)
{
	if (k < 0)
		return;
	expand(k);
	_data[k >> 3] |= (0x80 >> (k & 0x07));
}

void Bitmap::clear(int k)
{
	if (k < 0)
		return;
	expand(k);
	_data[k >> 3] &= ~(0x80 >> (k & 0x07));
}

bool Bitmap::test(int k)
{
    expand(k);
	if (k < 0 || k >= _size)
		return false;
	return _data[k >> 3] & (0x80 >> (k & 0x07));
}

void Bitmap::clearAll()
{
    int _bit_num = (_size + 7) >> 3;
    memset(_data, 0, _bit_num * sizeof(char));
}
