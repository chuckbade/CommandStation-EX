/*
 *  © 2021, Neil McKechnie. All rights reserved.
 *  
 *  This file is part of DCC++EX API
 *
 *  This is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  It is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with CommandStation.  If not, see <https://www.gnu.org/licenses/>.
 *  
 *  This class provides utilities for manipulating an array of bits.
 *  Provided by Harold Barth.  Modified by Chuck Bade
  */

class MiniBitSet {
private:
  uint8_t *data;
public:
  BitArray() {
  }

  void MiniBitSet::create(uint16_t n) {
    data = (uint8_t *)calloc((n+7)/8,1);
  }
  
  ~MiniBitSet() {
    free(data);
  }
  
  bool get(uint16_t n){
    return data[n/8] & (1<<n%8);
  }
  
  void set(uint16_t n) {
    data[n/8] |= (1<<n%8);
  }
  
  void clear(uint16_t n) {
    data[n/8] &= ~(1<<n%8);
  }
};

/* Example 
#define SIZE 17
int main() {
  MiniBitSet b(SIZE);
  b.set(0);
  b.set(SIZE/2);
  b.set(SIZE-1);
  b.set(SIZE-2);
  for (int n=0; n<SIZE; n++)
    printf("%c", b.get(n) ? '1' : '0');
  printf("\n");
  b.clear(SIZE-2);
  b.set(SIZE-3);
  for (int n=0; n<SIZE; n++)
    printf("%c", b.get(n) ? '1' : '0');
  printf("\n");
}
*/
