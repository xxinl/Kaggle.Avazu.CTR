
#ifndef _FTRL
#define _FTRL

#include <string>
#include <vector>
#include <unordered_map>
#include <utility>  
#include <stdlib.h>  
#include <math.h>
#include <algorithm>

#include "csv.h"

using std::string;

#pragma region col define

#define C_DAY 0
#define C_HOUR 1
#define C_C1 2
#define C_BANNER_POS 3
#define C_SITE_ID 4
#define C_SITE_DOM 5
#define C_SITE_CAT 6
#define C_APP_ID 7
#define C_APP_DOM 8
#define C_APP_CAT 9
#define C_DEV_ID 10
#define C_DEV_IP 11
#define C_DEV_MODEL 12
#define C_DEV_TYPE 13
#define C_DEV_CONN_TYPE 14
#define C_C14 15
#define C_C15 16
#define C_C16 17
#define C_C17 18
#define C_C18 19
#define C_C19 20
#define C_C20 21
#define C_C21 22
#define C_CLICK 23
#define C_ID 23

#pragma endregion col define


namespace cpp{

	class ftrl {

	private:
		const double _alpha;
		const double _beta;
		const double _l1;
		const double _l2;

		std::unordered_map<size_t, std::vector<double>> _wzn;
		std::unordered_map<size_t, double> _id_imp;

		bool _check_wzn_exist(const size_t xi){

			std::unordered_map<size_t, std::vector<double>>::const_iterator it_wzn = _wzn.find(xi);
			return it_wzn != _wzn.end();
		}

		void _check_insert_wzn(const size_t xi){

			std::unordered_map<size_t, std::vector<double>>::const_iterator it_wzn = _wzn.find(xi);
			if (it_wzn == _wzn.end()){

				std::vector<double> wzn = { 0, 0, 0 };
				_wzn.insert(std::make_pair(xi, wzn));
			}
		}

		void _gen_features(const std::vector<string>& x_raw, std::vector<std::pair<size_t, double>>& x, bool addImp);

		double _calc_logloss(const double p, const double y){

			double p2 = std::max(std::min(p, 1 - 10e-15), 10e-15);
			return y == 1 ? 0 - log(p2) : 0 - log(1 - p2);
		}

		void _train(const std::vector<string>& x_raw, bool addImp){

			double y = std::stod(x_raw[C_CLICK]);

			std::vector<std::pair<size_t, double>> x;
			_gen_features(x_raw, x, addImp);

			//predict
			double wTx = 0;
			for (std::vector<std::pair<size_t, double>>::const_iterator it_xi = x.begin(); it_xi != x.end(); ++it_xi)	{

				_check_insert_wzn(it_xi->first);

				std::vector<double>* wzn_i = &_wzn[it_xi->first];

				int sign = (*wzn_i)[1] < 0 ? -1 : 1;

				if (std::abs((*wzn_i)[1]) < _l1){

					(*wzn_i)[0] = 0;
				}
				else{

					(*wzn_i)[0] = (sign * _l1 - (*wzn_i)[1])
						/ ((_beta + std::sqrt((*wzn_i)[2])) / _alpha + _l2);

					wTx += (*wzn_i)[0] * it_xi->second;
				}
			}

			double p = 1 / (1 + std::exp(0 - std::max(std::min(wTx, 35.0), -35.0)));

			//update
			for (std::vector<std::pair<size_t, double>>::const_iterator it_xi = x.begin(); it_xi != x.end(); ++it_xi)	{

				std::vector<double>* wzn_i = &_wzn[it_xi->first];

				double g = (p - y) * it_xi->second;
				double sigma = (std::sqrt((*wzn_i)[2] + g*g) - std::sqrt((*wzn_i)[2])) / _alpha;
				(*wzn_i)[1] += g - sigma * (*wzn_i)[0];
				(*wzn_i)[2] += g*g;
			}
		}

	public:

		std::vector<int> f;
		std::vector<std::vector<int>> f2;

		ftrl(const double alpha = 0.05, const double beta = 1, const double l1 = 0.5, const double l2 = 1)
			: _alpha(alpha), _beta(beta), _l1(l1), _l2(l2){

			f = { C_DAY, C_HOUR, C_BANNER_POS, C_SITE_ID, C_SITE_CAT, C_APP_ID, C_APP_CAT,
				C_DEV_MODEL, C_C15, C_C16, C_C18, C_C19, C_C20, C_C21 };

			f2 = { { C_C1, C_DEV_TYPE }, { C_C14, C_C17 },
				{ C_BANNER_POS, C_SITE_ID }, { C_BANNER_POS, C_SITE_CAT }, { C_BANNER_POS, C_APP_ID },
				{ C_BANNER_POS, C_APP_CAT }, { C_BANNER_POS, C_DEV_MODEL }, { C_BANNER_POS, C_DEV_TYPE },
				{ C_DEV_MODEL, C_SITE_ID }, { C_DEV_MODEL, C_SITE_CAT }, { C_DEV_MODEL, C_APP_ID },
				{ C_DEV_MODEL, C_APP_CAT }, { C_DEV_TYPE, C_SITE_ID }, { C_DEV_TYPE, C_SITE_CAT }, { C_DEV_TYPE, C_APP_ID },
				{ C_DEV_TYPE, C_APP_CAT } };

			//cpp::csv id_imp_file("C:/Workspace/Kaggle/CRT/data/id_imp.csv");
			//std::vector<std::vector<string>> block_vec;
			//id_imp_file.read_chunk(1000000, block_vec);

			//std::hash<string> hash_fn;
			//for (std::vector<std::vector<string>>::iterator it(block_vec.begin());
			//	it != block_vec.end(); ++it)	{

			//	if (std::stoi((*it)[3]) < 100) break;

			//	_id_imp.insert(std::make_pair(hash_fn((*it)[0]), std::stod((*it)[4])));
			//}
		}

		double predict(const std::vector<string>& x_raw){

			std::vector<std::pair<size_t, double>> x;
			_gen_features(x_raw, x, true);

			double wTx = 0;
			for (std::vector<std::pair<size_t, double>>::const_iterator it_xi = x.begin(); it_xi != x.end(); ++it_xi)	{

				double wi = 0;
				if (_check_wzn_exist(it_xi->first))
					wi = _wzn[it_xi->first][0];

				wTx += wi * it_xi->second;
			}

			return 1 / (1 + std::exp(0 - std::max(std::min(wTx, 35.0), -35.0)));
		}

		void train(const std::vector<std::vector<string>>& block_vec, bool addImp){

			for (std::vector<std::vector<string>>::const_iterator it = block_vec.begin();
				it != block_vec.end(); ++it)	{

				_train(*it, addImp);
			}
		}

		double validate(const std::vector<std::vector<string>>& block_vec){
		
			double logloss = 0;
			for (std::vector<std::vector<string>>::const_iterator it = block_vec.begin();
				it != block_vec.end(); ++it)	{
				
				double p = predict(*it);
				logloss += _calc_logloss(p, std::stod((*it)[C_CLICK]));
			}

			return logloss;
		}
	};
}

#endif