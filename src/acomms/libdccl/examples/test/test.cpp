// t. schneider tes@mit.edu 11.20.09
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this software.  If not, see <http://www.gnu.org/licenses/>.


#include "acomms/dccl.h"
#include <iostream>
#include <cassert>

using dccl::operator<<;

void plus1(dccl::MessageVal& mv)
{
    long l = mv;
    ++l;
    mv = l;
}

void times2(dccl::MessageVal& mv)
{
    double d = mv;
    d *= 2;
    mv = d;
}

void prepend_fat(dccl::MessageVal& mv)
{
    std::string s = mv;
    s = "fat_" + s;
    mv = s;
}

void invert(dccl::MessageVal& mv)
{
    bool b = mv;
    b ^= 1;
    mv = b;
}

void algsum(dccl::MessageVal& mv, const std::vector<std::string>& params,
         const std::map<std::string,dccl::MessageVal>& vals)
{
    double d;
    // index 0 is the name ("sum"), so start at 1
    for(size_t i = 1, n = params.size(); i < n; ++i)
    {
        std::map<std::string, dccl::MessageVal>::const_iterator it = vals.find(params[i]);
        double v;
        if(it != vals.end() && it->second.get(v))
            d += v;
    }
    mv = d;
}


int main()
{
    std::cout << "loading xml file: test.xml" << std::endl;

    // instantiate the parser with a single xml file
    dccl::DCCLCodec dccl(DCCL_EXAMPLES_DIR "/test/test.xml", "../../message_schema.xsd");

    std::cout << dccl << std::endl;
    
    // load up the algorithms    
    dccl.add_algorithm("prepend_fat", &prepend_fat);
    dccl.add_algorithm("+1", &plus1);
    dccl.add_algorithm("*2", &times2);
    dccl.add_algorithm("invert", &invert);
    dccl.add_adv_algorithm("sum", &algsum);

    // must be kept secret!
    dccl.set_crypto_passphrase("my_passphrase!");
    
    std::map<std::string, dccl::MessageVal> in;
    
    bool b = true; 
    std::string e = "dog";      
    std::string s = "raccoon";  
    long i = 42;                
    double f = -12.5;           
    std::string h = "abcd1234"; 
    double sum = 0;             
    
    in["B"] = b;
    in["E"] = e;
    in["S"] = s;
    in["I"] = i;
    in["F"] = f;
    in["H"] = h;
    in["SUM"] = sum;

    std::string hex;
    std::cout << "sent values:" << std::endl 
              << in;

    dccl.encode(4, hex, in);

    std::cout << "hex out: " << hex << std::endl;
    hex.resize(32*2,'0');
    std::cout << "hex in: " << hex << std::endl;

    
    
    std::map<std::string, dccl::MessageVal> out;
    
    dccl.decode(4, hex, out);
    
    std::cout << "received values:" << std::endl 
              << out;    
    

    sum += i + f;
    ++i;

    dccl::MessageVal tmp = b;
    invert(tmp);
    b = tmp;
    
    tmp = s;
    prepend_fat(tmp);
    tmp.get(s);

    tmp = f;
    times2(tmp);
    f = tmp;
    
    assert(out["B"] == b);
    assert(out["E"] == e);
    assert(out["S"] == s);
    assert(out["F"] == f);
    assert(out["SUM"] == sum);
    assert(out["I"] == i);
    assert(out["H"] == h);
    
    std::cout << "all tests passed" << std::endl;
}