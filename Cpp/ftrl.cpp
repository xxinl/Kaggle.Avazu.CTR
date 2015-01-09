#include "stdafx.h"
#include "ftrl.h"


void cpp::ftrl::_gen_features(const std::vector<string>& x_raw, std::vector<std::pair<size_t, size_t>>& x, bool is_train){

	std::hash<string> hash_fn;

	int header_i = 0;
	for (int i = 0; i < x_raw.size(); ++i){

		if (i == 1 && is_train)
			continue;

		if (header_i != 1 && header_i != 5 && header_i != 8 && header_i != 10 && header_i != 11){

			if (!((x_raw[4 + is_train] == "85f751fd" && (header_i == 4 || header_i == 6))
				|| (x_raw[7 + is_train] == "ecad2386" && (header_i == 7 || header_i == 9)))){

				string f = string("F") + fheaders[header_i] + "_" + x_raw[i];
				x.push_back(std::make_pair(hash_fn(f), 1));
			}
		}

		header_i++;
	};

	//add hour
	string hour = x_raw[1 + is_train].substr(6, 2);
	string f = string("FHOUR") + "_" + hour;
	x.push_back(std::make_pair(hash_fn(f), 1));

	double y = is_train ? std::stod(x_raw[1]) : 0;
	//id imp and click
	_add_imp_f(x_raw[0], is_train, x, y, "F_IDIMP");
	//id imp and click
	_add_imp_f(x_raw[4+is_train], is_train, x, y, "F_SITEIMP");
	//id imp and click
	_add_imp_f(x_raw[7+is_train], is_train, x, y, "F_APPIMP");

	//interact
	const int f1[3] = { 3, 12, 13 };
	const int f2[6] = { 4, 6, 7, 9, 12, 13 };

	for (int i = 0; i < 3; ++i){

		for (int j = 0; j < 6; ++j){

			if (i > 0 && j > 3) continue;

			if (!(((f2[j] == 4 || f2[j] == 6) && x_raw[4 + is_train] == "85f751fd")
				|| ((f2[j] == 7 || f2[j] == 9) && x_raw[7 + is_train] == "ecad2386")))
			{
				string f = string("F") + fheaders[header_i] + "_" + x_raw[f1[i] + is_train] + "_" + x_raw[f2[j] + is_train];
				x.push_back(std::make_pair(hash_fn(f), 1));
			}
			header_i++;
		}
	}
}