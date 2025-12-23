#include <iostream>
#include <string>
#include <vector>
#include <fstream>

using namespace std;

enum class Color {red, green, blue};
enum class Size {small, medium, large};

class product {
public:
    product(const string& name, Color c, Size s) : name(name), color(c), size(s) {}
    string getName() {
        return name;
    }
    Color getColor() {
        return color;
    }
    Size getSize() {
        return size;
    }
private:
    string name;
    Color color;
    Size size;
};

class product_filter {
public:
    vector<product> filter_by_color(vector<product> products, Color c)
    {
        vector<product> items;
        for(auto& i:products)
        {
            if(i.getColor() == c)
                items.push_back(i);
        }
        return items;
    }
    vector<product> filter_by_size(vector<product> products, Size s)
    {
        vector<product> items;
        for(auto& i:products)
        {
            if(i.getSize() == s)
                items.push_back(i);
        }
        return items;
    }
    vector<product> filter_by_color_and_size(vector<product> products, Color c, Size s)
    {
        vector<product> items;
        for(auto& i:products)
        {
            if(i.getColor() == c && i.getSize() == s)
                items.push_back(i);
        }
        return items;
    }
};

int main()
{
    product apple{"Apple", Color::green, Size::small};
    product tree{"Tree", Color::green, Size::large};
    product house{"House", Color::blue, Size::large};
    
    vector<product> items{apple,tree,house};
    
    product_filter filter;
    vector<product> green_products = filter.filter_by_color(items, Color::green);
    for(auto& i: green_products)
        cout << i.getName() << endl;

    vector<product> large_products = filter.filter_by_size(items, Size::large);
    for(auto& i: large_products)
        cout << i.getName() << endl;

    vector<product> and_products = filter.filter_by_color_and_size(items, Color::green, Size::large);
    for(auto& i: and_products)
        cout << i.getName() << endl;

    return 0;
}

