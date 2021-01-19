#ifndef HMM_H
#define HMM_H



#include <iostream>
#include <vector>
#include <stdexcept>
#include <fstream>
#include <string>
#include <algorithm>
#include "util.h"

#include "Codebook.h"
#include "Frame.h"

class HMM{
private:
	Mat<ldouble> A,B;
	std::vector<ldouble> prior;
	int N, M;
public:
	HMM(int numStates=5, int nObservations=32, bool secondModel=false):
	  N(numStates), M(nObservations),
	  A(numStates, numStates, 0),
	  B(numStates, nObservations, 1.0/nObservations),
	  prior(numStates, 0){
		  prior[0] = 1.0;
		  //Inertia model
		  for(int i=0; i<N-1; i++){
			  if(secondModel){
				A[i][i] = 0.9;
				A[i][i+1] = 0.1;
			  }
			  else{
				  A[i][i] = 0.8;
				  A[i][i+1] = 0.2;
			  }
		  }
		  A[N-1][N-1] = 1.0;
	  }
	  //Default Construct
	  void setA(const Mat<ldouble>& newA){
		  A = newA;
	  }
	  void setB(const Mat<ldouble>& newB){
		  B = newB;
	  }

	ldouble evaluate(const std::vector<int>& observations){
		std::vector<ldouble> alphaCurr(N), alphaPrev(N);
		int T = observations.size();

		//dealing with underflows
		std::vector<ldouble> ci(T, 1.0);
		ldouble thres = 10E-100;
		ldouble logC=0;
		ldouble next_ct = 0;

		ldouble sum =0;
		//initialize
		for(int	i=0; i<N; i++){
			alphaPrev[i] = prior[i] * B[i][observations[0]];
			
			sum += alphaPrev[i];
		}
	
		next_ct = 1/sum;
		
		logC += log10(next_ct);
		
		for(int i=0; i<N; i++)
			alphaPrev[i] *= next_ct;
		
		


		//induction
		for(int i=1; i<T; i++){
			sum=0;
			for(int j=0; j<N; j++){
				ldouble s =0;
				for(int k=0; k<N; k++){
					s += alphaPrev[k] * A[k][j];
				}
				alphaCurr[j] = s * B[j][observations[i]];
				sum+= alphaCurr[j];
			}
			next_ct = 1/sum;
			
			alphaPrev = alphaCurr;
			for(int j=0; j<N; j++){
				alphaPrev[j] *= next_ct;
			}
			logC += log10(next_ct);
		}

		
		//termination
		ldouble ans = 0;
		sum=0;
		for(int i=0; i<N; i++){
			sum += alphaPrev[i];
		}
		ans = log10(sum) - logC;

		return ans;
	}

	ldouble custom_log(double val){
		if(val == 0 || val == -DBL_MAX) return -DBL_MAX;
		return log10(val);
	}

	std::pair<ldouble, std::vector<int>> alternate_viterbi(const std::vector<int>& observations){
		int T = observations.size();
		std::pair<ldouble, std::vector<int>> ans(-DBL_MAX, std::vector<int>(T));
		
		Mat<ldouble> delta(T, N);
		Mat<int> parents(T, N, -1);

		for(int i=0; i<N; i++)
			delta[0][i] = custom_log(prior[i]) + custom_log(B[i][observations[0]]);
		
		//induction
		for(int t = 1; t<T; t++){
			for(int i=0; i<N; i++){
				ldouble max = -DBL_MAX;
				int max_state = -1;
				for(int j=0; j<N; j++){
					ldouble val;
					if(delta[t-1][j] == -DBL_MAX || A[j][i] == 0){
						val = -DBL_MAX;
					}
					else
					  val  = delta[t-1][j]+custom_log(A[j][i]);
					
					if(val > max){
						max = val;
						max_state = j;
					}
				}
				if(max == -DBL_MAX || B[i][observations[t]]== 0){
					delta[t][i] = -DBL_MAX;
				}
				else
					delta[t][i] = max + custom_log(B[i][observations[t]]);
				parents[t][i] = max_state;
			}
		}

		//terminate and backpropogate
		int max_index = -1;
		for(int i=0; i<N; i++){
			if(delta[T-1][i] > ans.first){
				ans.first = delta[T-1][i];
				max_index = i;
			}
		}
			
		//most likely sequence
		ans.second[T-1] = max_index;
		for(int t=T-2; t>=0; t--){
			ans.second[t] = parents[t+1][ans.second[t+1]];
		}

		return ans;
	}
	
	std::pair<ldouble, std::vector<int>> viterbi(const std::vector<int>& observations){
		int T = observations.size();
		std::pair<ldouble, std::vector<int>> ans(-1, std::vector<int>(T));
		
		Mat<ldouble> delta(T, N);
		Mat<int> parents(T, N, -1);

		for(int i=0; i<N; i++)
			delta[0][i] = prior[i] * B[i][observations[0]];
		
		//induction
		for(int t = 1; t<T; t++){
			for(int i=0; i<N; i++){
				ldouble max = -1;
				int max_state = -1;
				for(int j=0; j<N; j++){
					if(delta[t-1][j]* A[j][i] > max){
						max = delta[t-1][j] * A[j][i];
						max_state = j;
					}
				}
				delta[t][i] = max* B[i][observations[t]];
				parents[t][i] = max_state;
			}
		}

		//terminate and backpropogate
		int max_index = -1;
		for(int i=0; i<N; i++){
			if(delta[T-1][i] > ans.first){
				ans.first = delta[T-1][i];
				max_index = i;
			}
		}
			
		//most likely sequence
		ans.second[T-1] = max_index;
		for(int t=T-2; t>=0; t--){
			ans.second[t] = parents[t+1][ans.second[t+1]];
		}

		return ans;
	}

	void alternate_re_estimation(const std::vector<int>& observations){
		//forward and backward procedures
		int T = observations.size();
		Mat<ldouble> alpha(T, N), beta(T, N);
		
		std::vector<ldouble> c(T, 1.0);
		ldouble scale_sum=0;

		//forward
		//initialize
		for(int i=0; i<N; i++){
			alpha[0][i] = prior[i] * B[i][observations[0]];
			scale_sum += alpha[0][i];
		}
		if(scale_sum != 0) c[0] = 1/scale_sum;
		for(int i=0; i<N; i++)
			alpha[0][i] *= c[0];

		//induction
		for(int t=1; t<T; t++){
			scale_sum=0;
			for(int i=0; i<N; i++){
			
				ldouble s = 0;
				for(int j=0; j<N; j++)
					s += alpha[t-1][j] * A[j][i];

				alpha[t][i] = s * B[i][observations[t]];
				scale_sum += alpha[t][i];
			}
			if(scale_sum != 0) c[0] = 1/scale_sum;
			for(int i=0; i<N; i++)
				alpha[t][i] *= c[t];
			
		}
	

		//backward
		for(int i=0; i<N; i++)
			beta[T-1][i] = 1.0 * c[T-1];
		//induction
		for(int t=T-2; t>=0; t--){
			for(int i=0; i<N; i++){
				beta[t][i] = 0.0;
				for(int j=0; j<N; j++){
					beta[t][i] += A[i][j] * B[j][observations[t+1]] * beta[t+1][j];
				}
				beta[t][i] *= c[t];
			}
		}

		//zeta
		std::vector<Mat<ldouble>> zeta(T, Mat<ldouble>(N, N));
		for(int t=0; t<T-1;t++){
			ldouble sum = 0;
			for(int i=0; i<N; i++){
				for(int j=0; j<N; j++){
					ldouble val = alpha[t][i] * A[i][j] * B[j][observations[t+1]] * beta[t+1][j];
					zeta[t][i][j] = val;
					sum += val;
				}
			}
			for(int i=0; i<N; i++){
				for(int j=0 ; j<N; j++){
					if(sum == 0) zeta[t][i][j] = 0;
					else zeta[t][i][j] /= sum;
				}
			}
		}

		Mat<ldouble> gamma(T, N);
		for(int t=0; t<T-1; t++){
			for(int i=0; i<N; i++){
				for(int j=0; j<N; j++)
					gamma[t][i] += zeta[t][i][j];
			}	
		}

		ldouble sum=0;
		

		//A
		for(int i=0; i<N; i++){
			for(int j=0; j<N; j++){
				ldouble numer = 0.0, denom =0.0;
				for(int t=0; t<T-1;t++){
					numer += zeta[t][i][j];
					denom += gamma[t][i];
				}
				if(denom == 0) A[i][j] = 0;
				else A[i][j] = numer/denom;
			}
		}
		//normalize
		for(int i=0; i<N; i++){
			sum=0;
			for(int j=0; j<N; j++){
				sum += A[i][j];
			}
			if(sum==0){
				if(i<N-1){
					A[i][i] = 0.8;
					A[i][i+1] = 0.2;
				}
				else A[i][i] = 1.0;
				continue;
			}
			for(int j=0; j<N; j++)
				A[i][j] /= sum;
		}

		//B
		for(int i=0; i<N; i++){
			for(int j=0; j<M; j++){
				ldouble num=0, denom=0;
				for(int t=0; t<T-1; t++){
					if(observations[t] == j) num += gamma[t][i];
					denom += gamma[t][i];
				}
				if(denom == 0) B[i][j] = 0;
				else B[i][j] = num/denom;
				if(B[i][j] < 10E-30) B[i][j] = 10E-30;
			}
		}
		//normalize
		for(int i=0; i<N; i++){
			sum=0;
			for(int j=0; j<M; j++){
				sum += B[i][j];
			}
			for(int j=0; j<M; j++)
				B[i][j] /= sum;
			
			
		}
	}

	void re_estimation(const std::vector<int>& observations){
		//forward and backward procedures
		int T = observations.size();
		Mat<ldouble> alpha(T, N), beta(T, N);
	
		//forward
		//initialize
		for(int i=0; i<N; i++){
			alpha[0][i] = prior[i] * B[i][observations[0]];
		}

		//induction
		for(int t=1; t<T; t++){
			for(int i=0; i<N; i++){
			
				ldouble s = 0;
				for(int j=0; j<N; j++)
					s += alpha[t-1][j] * A[j][i];

				alpha[t][i] = s * B[i][observations[t]];
			}
		}
	

		//backward
		for(int i=0; i<N; i++)
			beta[T-1][i] = 1.0;
		//induction
		for(int t=T-2; t>=0; t--){
			for(int i=0; i<N; i++){
				beta[t][i] = 0.0;
				for(int j=0; j<N; j++){
					beta[t][i] += A[i][j] * B[j][observations[t+1]] * beta[t+1][j];
				}
			}
		}

		//zeta
		std::vector<Mat<ldouble>> zeta(T, Mat<ldouble>(N, N));
		for(int t=0; t<T-1;t++){
			ldouble sum = 0;
			for(int i=0; i<N; i++){
				for(int j=0; j<N; j++){
					ldouble val = alpha[t][i] * A[i][j] * B[j][observations[t+1]] * beta[t+1][j];
					zeta[t][i][j] = val;
					sum += val;
				}
			}
			for(int i=0; i<N; i++){
				for(int j=0 ; j<N; j++){
					if(sum == 0) zeta[t][i][j] = 0;
					else zeta[t][i][j] /= sum;
				}
			}
		}

		Mat<ldouble> gamma(T, N);
		for(int t=0; t<T-1; t++){
			for(int i=0; i<N; i++){
				for(int j=0; j<N; j++)
					gamma[t][i] += zeta[t][i][j];
			}	
		}

		ldouble sum=0;
		

		//A
		for(int i=0; i<N; i++){
			for(int j=0; j<N; j++){
				ldouble numer = 0.0, denom =0.0;
				for(int t=0; t<T-1;t++){
					numer += zeta[t][i][j];
					denom += gamma[t][i];
				}
				if(denom == 0) A[i][j] = 0;
				else A[i][j] = numer/denom;
			}
		}
		//normalize
		for(int i=0; i<N; i++){
			sum=0;
			for(int j=0; j<N; j++){
				sum += A[i][j];
			}
			if(sum==0) continue;
			for(int j=0; j<N; j++)
				A[i][j] /= sum;
		}

		//B
		for(int i=0; i<N; i++){
			for(int j=0; j<M; j++){
				ldouble num=0, denom=0;
				for(int t=0; t<T-1; t++){
					if(observations[t] == j) num += gamma[t][i];
					denom += gamma[t][i];
				}
				if(denom == 0) B[i][j] = 0;
				else B[i][j] = num/denom;
				if(B[i][j] < 10E-30) B[i][j] = 10E-30;
			}
		}
		//normalize
		for(int i=0; i<N; i++){
			sum=0;
			for(int j=0; j<M; j++){
				sum += B[i][j];
			}
			for(int j=0; j<M; j++)
				B[i][j] /= sum;
			
		}
	}
	
	//helpers
	//Function which trains on a dataset and returns a HMM using the restimation procedure
	static HMM train(int numData, int trainPerData, int numIterations, int N, Codebook& cb, const std::string& path, const std::string& word, bool trim = true){

			std::stringstream ss;
			int M = cb.size();

			std::vector<HMM> hmms;
			hmms.push_back(HMM(N, M, false));
			hmms.push_back(HMM(N, M, true));
	
			std::vector<std::pair<ldouble, HMM>> next_hmms;

			for(int k=1; k<=numIterations; k++){
				next_hmms.clear();

				ldouble s = 0;
				for(int i=0; i<numData; i++){
					ss.str(std::string());
					//ss << path << "\\194101013_" << word << "_" << i << ".txt";
					ss << path << "\\" << i << "_" << word << ".txt";

					auto data = getCepstrals(ss.str(), trim);
		
					std::vector<int> obs;
					for(int i=0; i<data.rows(); i++){
						int ob = cb.getNearest(data[i]);
						obs.push_back(ob);
					}
					std::vector<HMM> hmm_curr(hmms);
			

					for(int h=0; h<(int)hmm_curr.size(); h++){			
						for(int j=0; j<trainPerData; j++)
								hmm_curr[h].alternate_re_estimation(obs);
						auto& p = hmm_curr[h].alternate_viterbi(obs);
						s += p.first;
						next_hmms.push_back(std::pair<ldouble, HMM>(p.first , hmm_curr[h]));
					}
					std::cout << ss.str() << "\n";
				}
				
				std::sort(next_hmms.begin(), next_hmms.end(), [](const std::pair<ldouble, HMM>&a,const std::pair<ldouble, HMM>&b){
					return a.first > b.first;
				});

				Mat<ldouble> A(N, N), B(N, M);
				int top = (int)(next_hmms.size());
				for(int i=0; i<top; i++){
					A = A+ next_hmms[i].second.getA();
					B = B+ next_hmms[i].second.getB();
				}
				A.divide(top);
				B.divide(top);
				HMM finalHMM;
				finalHMM.setA(A);
				finalHMM.setB(B);
				hmms.clear();
				hmms.push_back(finalHMM);
				std::cout << "ITERATION " << k << " DONE\n";
				std::cout << s << "\n";
			}

			return hmms[0];
	}
	
	
	static int test(const std::vector<std::string>& words, std::vector<HMM>& hmms, Codebook& cb, 
		const std::string& path, const std::string& word, int numData, int offset=0, bool trim = true){

		std::stringstream ss;
		int nTest=0;
		int corr=0;
		int W = words.size();

	
		for(int occ=offset; occ<offset+numData; occ++){
				ss.str(std::string());
				//ss << path << "\\194101013_" << word << "_" << occ <<".txt";
				ss << path << "\\" << occ << "_" << word << ".txt";
				std::cout << ss.str() << " ";
				auto data = getCepstrals(ss.str(), trim);

				std::vector<int> obs;
				for(int i=0; i<data.rows(); i++){
					int ob = cb.getNearest(data[i]);
					obs.push_back(ob);
					
				}

				//check best hmm along all the hmms
				int best=-1;
				ldouble max=-DBL_MAX;
				for(int j=0; j<W; j++){
						auto val = hmms[j].evaluate(obs);
						if(val > max){
							max = val;
							best = j;
						}
				}
				corr += (word == words[best]);
				nTest++;
				std::cout << word << " predicted as: " << words[best] << "\n";
		}
		
		std::cout << "Correct:" << (ldouble)corr/nTest << "\n";
		return corr;

	}
	
	static int predict(const std::vector<std::string>& words, std::vector<HMM>& hmms, Codebook& cb, const std::string& file) {

	
		auto data = getCepstrals(file);
		int W = words.size();

		std::vector<int> obs;
		for (int i = 0; i < data.rows(); i++) {
			int ob = cb.getNearest(data[i]);
			obs.push_back(ob);

		}

		//check best hmm along all the hmms
		int best = -1;
		ldouble max = -DBL_MAX;
		for (int j = 0; j < W; j++) {
			auto val = hmms[j].alternate_viterbi(obs).first;
			if (val > max) {
				max = val;
				best = j;
			}
		}
		return best;
	}

	static int predict(const std::vector<std::string>& words, std::vector<HMM>& hmms, Codebook& cb, Uint8* buffer, int len) {


		auto data = getCepstrals(buffer, len);
		int W = words.size();

		std::vector<int> obs;
		for (int i = 0; i < data.rows(); i++) {
			int ob = cb.getNearest(data[i]);
			obs.push_back(ob);

		}

		//check best hmm along all the hmms
		int best = -1;
		ldouble max = -DBL_MAX;
		for (int j = 0; j < W; j++) {
			auto val = hmms[j].alternate_viterbi(obs).first;
			if (val > max) {
				max = val;
				best = j;
			}
			std::cout << val << " ";
		}
		return best;
	}

	Mat<ldouble>& getA(){
		return A;
	}
	Mat<ldouble>& getB(){
		return B;
	}
	void printState(){
		A.print();
		B.print();
	}

	void saveState(const std::string& name){
		std::ofstream file(name);
		if(file.is_open()){
			//save A
			for(int i=0; i<N; i++){
				for(int j=0; j<N; j++){
					file << A[i][j] << " ";
				}
				file << "\n";
			}
			//save B
			for(int i=0; i<N; i++){
				for(int j=0; j<M; j++)
					file << B[i][j] << " ";
				file << "\n";
			}

			//save PI
			for(int i=0; i<N; i++){
					file << prior[i] << " ";
			}



			file.close();
		}
		else{
			std::cerr << name << " File Can't be opened!\n";
			throw std::exception("FILE NOT FOUND");
		}
	}

	void readState(const std::string& path, int N, int M){
		std::ifstream file(path);
		if(file.is_open()){
			Mat<ldouble> A(N, N), B(N, M);
			std::vector<ldouble> PI(N);

			//read A
			for(int i=0; i<N; i++){
				for(int j=0; j<N; j++)
					file >> A[i][j];
			}
			//read B
			for(int i=0; i<N; i++){
				for(int j=0; j<M; j++)
					file >> B[i][j];
			}
			//read PI
			for(int i=0; i<N; i++)
				file >> PI[i];
			//set A and B
			this->A = A;
			this->B = B;
			this->prior = PI;

			file.close();
		}
		else{
			std::cerr << path << " File Can't be opened!\n";
			throw std::exception("FILE NOT FOUND");
		}

	}
};

#endif