#ifndef _CSV
#define _CSV

#include <string>
#include <fstream>
#include <vector>
#include <iostream>

#include <boost/tokenizer.hpp>

using std::string;

namespace cpp{

	class csv {
	private:
		std::ifstream _file;

	public:
		csv(string path, bool header = true) : _file(path){	
		
			if (header){

				string line;
				std::getline(_file, line);
			}
		}

		~csv(){

			close();
		}

		void close(){
		
			if (_file.is_open())
				_file.close();
		}
		
		bool read_chunk(size_t no_lines, std::vector<std::vector<string>>& block_vec){	
		
			string line;

			size_t report_size = no_lines / 10;
			size_t count = 0;
			while (count < no_lines && std::getline(_file, line)) {

				if (count % report_size == 0){

					std::cout << count / 1000 << "k ";
				}

				boost::tokenizer<boost::escaped_list_separator<char> > tk(
					line, boost::escaped_list_separator<char>('\\', ',', '\"'));

				std::vector<string> row_vec;
				int col_i = 0;
				for (boost::tokenizer<boost::escaped_list_separator<char>>::iterator it(tk.begin());
					it != tk.end(); ++it)	{

					row_vec.push_back(*it);
				}

				block_vec.push_back(row_vec);

				count++;
			}

			return block_vec.size() > 0;
		}
	};
}

#endif