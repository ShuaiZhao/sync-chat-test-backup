#include <iostream>
#include <string>
#include <fstream>

int main()
{
        using namespace std;

    ifstream file("/home/shuai/workspace/ndnsim-sync-chat/sync-chat/results/log/gop_size.txt");
    if(file.is_open())
    {
        string myArray[10000];
        for(int i = 0; i < 5; ++i)
        {
            file >> myArray[i];
            cout << myArray[i]<< endl;
         }
     }

}
