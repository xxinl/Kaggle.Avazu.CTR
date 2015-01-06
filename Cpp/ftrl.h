
#ifndef _FTRL
#define _FTRL

#include <string>
#include <vector>
#include <unordered_map>
#include <utility>  
#include <stdlib.h>  
#include <math.h>
#include <algorithm>

using std::string;

#define D pow(2, 29)
#define NO_FEATURE 23
#define HOLD_OUT 8

namespace cpp{

	class ftrl {

	private:
		const double _alpha;
		const double _beta;
		const double _l1;
		const double _l2;

		const string fheaders[NO_FEATURE];

		double _logloss;
		std::unordered_map<size_t, std::vector<double>> _wzn;
		std::unordered_map<size_t, std::pair<size_t, size_t>> _id_imp_cl;

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

		void _gen_features(const std::vector<string>& x_raw, std::vector<std::pair<size_t, size_t>>& x, bool is_train){

			std::hash<string> hash_fn;

			size_t id_hash = hash_fn(x_raw[0]);
			std::unordered_map<size_t, std::pair<size_t, size_t>>::iterator it_id_imp_cl = _id_imp_cl.find(id_hash);
			bool is_new = it_id_imp_cl == _id_imp_cl.end();
			if (is_new){

				x.push_back(std::make_pair(hash_fn("F_NEW"), 1));
				_id_imp_cl.insert(std::make_pair(id_hash, std::make_pair(0, 0)));
			}
			else{

				x.push_back(std::make_pair(hash_fn("F_NEW"), 0));
			}


			for (int i = 0; i < x_raw.size(); ++i){

				if (i == 1 && is_train)
					continue;

				int header_i = i;
				if (is_train && i > 1)
					header_i = i - 1;
				
				if (header_i != 5 && header_i != 8 && x_raw[i] != "d41d8cd9"){

					string f = string(string("F") + fheaders[header_i] + "_" + x_raw[i]);
					x.push_back(std::make_pair(hash_fn(f), 1));
					if (is_new && header_i > 0 && header_i < 15){

						x.push_back(std::make_pair(hash_fn(f+"_F_NEW"), 1));
					}
					else{

						x.push_back(std::make_pair(hash_fn(f + "_F_EXIST"), 1));
					}
				}
			};


			std::pair<size_t, size_t>* imp_cl = &_id_imp_cl[id_hash];

			x.push_back(std::make_pair(hash_fn("F_IMP"), imp_cl->first/100000));
			x.push_back(std::make_pair(hash_fn("F_CL"), imp_cl->second/1000));

			imp_cl->first += 1;
			if (is_train)
				imp_cl->second += std::stod(x_raw[1]);
		}

		double _calc_logloss(const double p, const double y){

			double p2 = std::max(std::min(p, 1 - 10e-15), 10e-15);
			return y == 1 ? 0 - log(p2) : 0 - log(1 - p2);
		}

		void _train(const std::vector<string>& x_raw){

			double y = std::stod(x_raw[1]);

			std::vector<std::pair<size_t, size_t>> x;
			_gen_features(x_raw, x, true);

			//predict
			double wTx = 0;
			for (std::vector<std::pair<size_t, size_t>>::const_iterator it_xi = x.begin(); it_xi != x.end(); ++it_xi)	{

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
			for (std::vector<std::pair<size_t, size_t>>::const_iterator it_xi = x.begin(); it_xi != x.end(); ++it_xi)	{

				std::vector<double>* wzn_i = &_wzn[it_xi->first];

				double g = (p - y) * it_xi->second;
				double sigma = (std::sqrt((*wzn_i)[2] + g*g) - std::sqrt((*wzn_i)[2])) / _alpha;
				(*wzn_i)[1] += g - sigma * (*wzn_i)[0];
				(*wzn_i)[2] += g*g;
			}
		}

	public:

		ftrl(const double alpha = 0.1, const double beta = 1, const double l1 = 1, const double l2 = 1)
			: _alpha(alpha), _beta(beta), _l1(l1), _l2(l2), _logloss(0){

			//_wzn.reserve(D);
		}

		double predict(const std::vector<string>& x_raw, bool is_validate){

			std::vector<std::pair<size_t, size_t>> x;
			_gen_features(x_raw, x, is_validate);

			double wTx = 0;
			for (std::vector<std::pair<size_t, size_t>>::const_iterator it_xi = x.begin(); it_xi != x.end(); ++it_xi)	{

				double wi = 0;
				if (_check_wzn_exist(it_xi->first))
					wi = _wzn[it_xi->first][0];

				wTx += wi * it_xi->second;
			}

			return 1 / (1 + std::exp(0 - std::max(std::min(wTx, 35.0), -35.0)));
		}

		double train(const std::vector<std::vector<string>>& block_vec){

			int count = 2;
			for (std::vector<std::vector<string>>::const_iterator it = block_vec.begin();
				it != block_vec.end(); ++it)	{

				if (count % HOLD_OUT == 0 || count % HOLD_OUT == 1){

					double p = predict(*it, true);
					_logloss += _calc_logloss(p, std::stod((*it)[1]));
				}
				else
					_train(*it);

				count++;
			}

			return _logloss / (count - 2);
		}

		double get_logloss() const{

			return _logloss;
		}
	};
}

#endif