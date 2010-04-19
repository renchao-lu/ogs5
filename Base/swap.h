#ifndef SWAP_H_
#define SWAP_H_

/**
 * swap the content of arg0 and arg1
 */
template<class T> void swap(T& arg0, T& arg1)
{
  T temp(arg0);
  arg0 = arg1;
  arg1 = temp;
}

#endif //SWAP_H_
