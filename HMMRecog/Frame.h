#ifndef FRAME_H
#define FRAME_H


#include "util.h"
#include "HMM.h"
#include "Codebook.h"
#include <SDL/SDL.h>
const int FRAME_SIZE = 320;
const int P = 12;
const ldouble WORD_START_THRESHOLD = 10.0;
const ldouble SILENCE_THRESH = 100000;
const int AVG_OVER = 6;

struct Frame{

	std::vector<ldouble> samples; 
	std::vector<ldouble> r; //vector for all the r's R(0)....R(12)
	std::vector<ldouble> a; //vector for Linear predictor coefficients
	std::vector<ldouble> c; //cepstral coefficients

	//method to calculate ai's, given ri's
	std::vector<ldouble> levinsion_durbin(std::vector<ldouble> r){

	int p = r.size()-1;
	
	std::vector<ldouble> E(p+1), K(p+1);
	std::vector<std::vector<ldouble>> a(p+1, std::vector<ldouble>(p+1));

	E[0] = r[0];

	for(int i=1; i<=p ;i++){
		if(i==1){
			K[i] = r[i]/E[i-1];
		}
		else{
			ldouble summation=0;
			for(int j=1; j<=i-1; j++)
				summation += a[j][i-1] * r[i-j];
			K[i] = (r[i] - summation)/E[i-1];
		}
		a[i][i] = K[i];
		for(int j=1; j<=i-1; j++)
			a[j][i] = a[j][i-1] - K[i]*a[i-j][i-1];
		E[i] = (1.0 - K[i]* K[i]) * E[i-1];
	}


	std::vector<ldouble> ans;
	for(int i=1; i<=p; i++)
		ans.push_back(a[i][p]);
	return ans;
}

	//method to calculate ai's and ci's for the frame
	void calculateCoefficients(){
		
		//first calculate R(0)...R(k)...R(12)
		for(int k=0; k<= P; k++){
			ldouble sum=0;
			int no_samples = samples.size();

			//iterate over all samples of frame
			for(int j=0; j<no_samples-k; j++)
				sum += samples[j] * samples[j+k];
			
			r.push_back(sum);
		}
		//STE cutoff, R(0). If energy less than threshold, don't calculate ai's
		/*if(r[0] < 5000000){
				return;
		}*/

		
		//calculate ai's using levinson and store in vector a
		a = levinsion_durbin(r);

		//calculate cepstral coefficients
		ldouble c0 = log( r[0] * r[0] );

		for(int i=1; i<=P; i++){
			ldouble sum=0;
			
			for(int k=1; k<=i-1; k++){
				sum += ((ldouble)k/i) * c[k-1] * (a[i-k-1]);
			}
			c.push_back( (a[i-1]) + sum);
		}
	}	

	//method to apply raised sine window to calculted ci's
	void apply_raised_sin_window(){
		
		for(int i=0; i<P; i++){
			ldouble raised_sin_window_weight = 1 + (P/2.0) * sin( (3.14159 * (i+1)) / P );
			c[i] *= raised_sin_window_weight; 
		}

	}
	//return STE of the frame
	ldouble getSTE(){
		ldouble sum=0;
		int n_samples = samples.size();
		for(int i=0; i<n_samples; i++)
			sum += samples[i]* samples[i];
		return sum/n_samples;
	}

};

//dc shift correction and normalize samples
void preprocess(std::vector<ldouble>& samples){
		
		
		//calculate DC bias
		//initial 3 frames are Silence
		int n_samples = samples.size();

		ldouble dc_shift = 0;
		for(int i=0; i < FRAME_SIZE*2; i++)
			dc_shift += samples[i];
		
		dc_shift /= (FRAME_SIZE*2);
		//apply correction and get max samples value
		
		ldouble max_sample = abs(samples[0] - dc_shift);
		for(int i=0; i<n_samples; i++){
			samples[i] -= dc_shift;
			if(abs(samples[i]) > max_sample)
				max_sample = abs(samples[i]);
		}
		
		//normlization
		//New MAX peak is 10,000 
		ldouble normalization_factor = 10000.0/max_sample;
		for(int i=0; i<n_samples; i++){
			samples[i] = samples[i] * normalization_factor;
		}
		
}


Mat<ldouble> getCepstrals(std::string fileName, bool trim = true){

	std::ifstream file(fileName);

	if(file.is_open()){
		//read file
		std::vector<ldouble> samples;
		while(!file.eof()){
			ldouble sam;
			file >> sam;
			samples.push_back(sam);
		}
		file.close();

		preprocess(samples);

		//get steady frames from steady part
		//windowing and framing
		std::vector<Frame> frames;
		//hamming window
		std::vector<ldouble> hamming(FRAME_SIZE);

		for(int j=0; j<FRAME_SIZE; j++)
			hamming[j] = 0.54 - 0.46*cos( (2*3.14159*j)/(FRAME_SIZE-1));

		//sliding window
		//skip silent frames
		int n_samples = samples.size();

		int startSam = 0;
		if (trim) {
			/*ldouble prev = DBL_MAX;
			for (int j = 0; j + FRAME_SIZE < n_samples; j += FRAME_SIZE) {
				Frame frame;
				for (int k = j; k < j + FRAME_SIZE; k++)
					frame.samples.push_back(samples[k] * hamming[k - j]);

				ldouble curr = frame.getSTE();

				if (curr / prev > WORD_START_THRESHOLD) {
					int s = j - FRAME_SIZE/3;
					startSam = s >= 0 ? s : 0;
					break;
				}
				prev = curr;
			}*/
			//method 2
			//keep track of avg and check threshold
			ldouble avg_ste = 0;
			int avg_over = 3, cnter = 0;
			for (int j = 0; j + FRAME_SIZE < n_samples; j += FRAME_SIZE) {
				Frame frame;
				for (int k = j; k < j + FRAME_SIZE; k++)
					frame.samples.push_back(samples[k] * hamming[k - j]);
				avg_ste += frame.getSTE();
				if (cnter % avg_over == 0) {
					avg_ste /= avg_over;
					if (avg_ste > SILENCE_THRESH*10) {
						//found starting
						startSam = j - (avg_over+2) * FRAME_SIZE;
						startSam = (startSam < 0) ? 0 : startSam;
						break;
					}
					avg_ste = 0;
				}
				cnter++;
			}
		}
		int no_frames = 0;
		ldouble avg_curr_ste = 0;
		int cnt = 0;
		
		for(int j=startSam; j+FRAME_SIZE < n_samples; j += FRAME_SIZE/3){
				Frame frame;
				
				//apply hamming window accordingly for the frame i
				ldouble curr_ste = 0;
				for (int k = j; k < j + FRAME_SIZE; k++) {
					frame.samples.push_back(samples[k] * hamming[k - j]);
					curr_ste += samples[k] * samples[k];
				}
				curr_ste /= FRAME_SIZE;
				avg_curr_ste += curr_ste;
				if (cnt % AVG_OVER == 0) {
					avg_curr_ste /= AVG_OVER;
					
					if (frames.size() > 50 && avg_curr_ste < SILENCE_THRESH) {
						std::cout << startSam << " " << j << "\n";
						break;
					}
					
					avg_curr_ste = 0;
				}
				cnt++;
				frames.push_back(frame);
				
		}
		no_frames = frames.size();
		std::cout << "Frames: " << no_frames << "\n";
		
		Mat<ldouble> ans(no_frames,P);
		for(int i=0; i<no_frames; i++){
			frames[i].calculateCoefficients();
			frames[i].apply_raised_sin_window();

			for(int j=0; j<P; j++){
				ans[i][j] = frames[i].c[j];
			}
		}
		return ans;
	}
	else{
		std::cout << "ERROR::" << fileName << "\n";
		exit(-1);
	}

}

//read from buffer
Mat<ldouble> getCepstrals(Uint8* buffer, int len) {
	
		

		std::vector<ldouble> samples;
		for (int i = 50; i < len; i += 2) {
			int sam = *((Sint16*)(buffer+i));
			samples.push_back(sam);
		}
		preprocess(samples);
	

		//get steady frames from steady part
		//windowing and framing
		std::vector<Frame> frames;
		//hamming window
		std::vector<ldouble> hamming(FRAME_SIZE);

		for (int j = 0; j < FRAME_SIZE; j++)
			hamming[j] = 0.54 - 0.46 * cos((2 * 3.14159 * j) / (FRAME_SIZE - 1));

		//sliding window
		int n_samples = samples.size();
		int no_frames = 0;
		//skip silent frames
		int startSam = 0;
		ldouble prev = DBL_MAX;
		for (int j = 0; j + FRAME_SIZE < n_samples; j += FRAME_SIZE) {
			Frame frame;
			for (int k = j; k < j + FRAME_SIZE; k++)
				frame.samples.push_back(samples[k] * hamming[k - j]);
			ldouble curr = frame.getSTE();
			if (curr / prev > WORD_START_THRESHOLD) {
				int s = j - 1 * FRAME_SIZE;
				startSam = s >= 0 ? s : 0;
				break;
			}
			prev = curr;
		}

		std::ofstream f("test.txt");

		ldouble avg_curr_ste = 0;
		int cnt = 0;
		for (int j = startSam; j + FRAME_SIZE < n_samples; j += FRAME_SIZE / 3) {
			Frame frame;
			
			for (int k = j; k < j + FRAME_SIZE / 3; k++)
				f << samples[k] << "\n";

			//apply hamming window accordingly for the frame i
			ldouble curr_ste = 0;
			for (int k = j; k < j + FRAME_SIZE; k++) {
				frame.samples.push_back(samples[k] * hamming[k - j]);
				curr_ste += samples[k] * samples[k];
			}
			curr_ste /= FRAME_SIZE;
			avg_curr_ste += curr_ste;
			if (cnt % AVG_OVER == 0) {
				avg_curr_ste /= AVG_OVER;

				if (frames.size() > 50 && avg_curr_ste < SILENCE_THRESH) {
					std::cout << startSam << " " << j + FRAME_SIZE << "\n";
					break;
				}
				avg_curr_ste = 0;
			}
			cnt++;
			frames.push_back(frame);
		}
		f.close();
		no_frames = frames.size();
		//std::cout << no_frames << "\n";

		Mat<ldouble> ans(no_frames, P);
		for (int i = 0; i < no_frames; i++) {
			frames[i].calculateCoefficients();
			frames[i].apply_raised_sin_window();

			for (int j = 0; j < P; j++) {
				ans[i][j] = frames[i].c[j];
			}
		}
		
		return ans;
}
#endif

