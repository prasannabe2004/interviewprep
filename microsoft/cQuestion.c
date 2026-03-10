#include <stdio.h>
#include<string.h>
 
#define MAX_SIZE 1024
int main (int argc, char *argv[])
{
char* str = NULL, *str1 = NULL;
if(sizeof(argv)<2)
	return -1;
	
int len = 0;
str = argv[1];
len =  findlength (str);
 
if (len == 0)
{
  return -1;
}

printf ("length of input string is %d", len);
str1 =  AddnewlinetoString(str);
if(!str1)
	return -2;
printf ("New string is %s ", str1);
free(str1);

}
 "a\n0"
  01
    
  len=2;
char* AddnewlinetoString (char *s)
{
  if(!s)
	return NULL;
	
  char buffer=(char)malloc(1024*sizeof(char));
  if(buffer)
		return NULL;
		
  strcpy(buffer,s);	
 // buffer[strlen(s)-1] = '\n';
 buffer[strlen(s)]='\n';
 buffer[strlen(s)+1]=0;
 
  return buffer;
}

int findlength (char* s)
{
	if(!s)
		return 0;
		
	int counter=-1;
	while(counter< min((int)strlen(s), MAX_SIZE)) 
		counter++;
		
	return counter;
}

Base *pB = &derived_obj; // Class Derived : Base ...
 
pB->func()
 
//func() is virtual in both base and derived
 
 
 Given a sequence of 2D transformation strings (e.g., ["SCALE", "ROTATE_90", "ROTATE_90_INV", "TRANSLATE", "SCALE_INV"]), 
 simplify the sequence by removing adjacent pairs that are inverses of each other (e.g., X followed by X_INV).
 Implement an algorithm to return the simplified, canonical sequence.
 
 ["SCALE","ROTATE_90_INV", "ROTATE_90", "TRANSLATE", "SCALE_INV"]),
 
 S, S_INV, S_INV, S,S, HHF, HHF_INV, S_INV, S_INV
 S_INV
 
 SCALE_INV
 TRANSLATE
 scale
 
 
 Implement a class to manage a one-dimensional canvas (a line segment). It must support up to 10^5 operations.
PAINT(l, r): Marks the range [l, r) as painted.
ERASE(l, r): Marks the range [l, r) as erased.
QUERY(): Returns the total length of the painted sections.


 P 1, 5
    {8,10}
	13, 15
	4,6
 E 3 4
 
 [1,2]P
 [3,4] E
  [5,5] P
 
 set<pair<int,int>>st;
 PAINT{
	
	int start, end;
	
	st.insert({start,0});
	st.insert({end,1})
	
	set<pair<int,int>>::iterator it;
	it=st.lower_bound({start,0});
 }
 
 PRINT(int start, int end){
	set<pair<int,int>>::iterator it,it2;
	it=st.lower_bound({start,0});
	
	if(*it.second==1)
	{
		1)and insert and new pair witt ending at it start-1
		2) Move ahead and delete all overapplying ranges till end , and in the end add {end,1}
		3) delete the itr ensuring that you don't access again
	}
	else
	{
		1) Keep that iteator
		2) ove ahead and delete all overapplying ranges till end , and in the end add push {end,1}
	}
 }
 
 
 1,0
 
 3,1
 4,0
 6,1
 
 8,0,
 10,1
 13,0
 15,1