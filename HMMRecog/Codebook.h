#ifndef CODEBOOK_H
#define CODEBOOK_H

#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

//A codebook class
class Codebook{
private:
	std::vector<std::vector<ldouble>> codebook; //codebook entries
public:

	//method to add codebook entry
	void addEntry(const std::vector<ldouble>& v){
		codebook.push_back(v);
	}

	//method to split codebook entries using epsilon
	void splitCodebook(ldouble epsilon){

		std::vector<std::vector<ldouble>> newEntries;

		int size=codebook.size();
		for(int i=0; i<size; i++){
			std::vector<ldouble>& entry = codebook[i]; //get entry
			std::vector<ldouble> s1, s2;

			int k = entry.size();
			//create new two vectors
			for(int i=0; i<k; i++){
				s1.push_back(entry[i] * (1.0 - epsilon));
				s2.push_back(entry[i] *  (1.0 + epsilon));
			}

			//push into new entries
			newEntries.push_back(s1);
			newEntries.push_back(s2);
		}
		//update the codebook
		codebook = newEntries;
	}
	//method to delete codebook entry at some index
	void deleteEntry(int index){
		int size = codebook.size();
		if(index < 0 || index >= size) return;
		codebook.erase( codebook.begin() + index);
	}
	

	//get a codebook entry reference
	std::vector<ldouble>& operator[](int index){
		int size = codebook.size();
		if(index < 0 || index >= size) throw std::out_of_range("Invalid Index");
		return codebook[index];
	}
	//get distance between 2 codevectors
	//distance is tokhura distance
	ldouble distance(const std::vector<ldouble>& x, const std::vector<ldouble>& y){
		ldouble tokhura_weights[12] = {1.0, 3.0, 7.0, 13.0, 19.0, 22.0, 25.0, 33.0, 42.0, 50.0, 56.0, 61.0};
		ldouble ans = 0;
		int k = x.size();
		for(int i=0; i<k; i++)
			ans += tokhura_weights[i] * pow(x[i] - y[i],2);
		return ans;
	}

	//calculates average distortion for the given dataSet
	ldouble distortion(const std::vector<std::vector<ldouble>>& data){
		//average distortion
		int M = data.size();
		ldouble ans=0;

		for(int i=0; i<M; i++){
			int e = getNearest(data[i]);
			ans += distance(codebook[e], data[i]);
		}
		return ans/M;
	}

	//gets the nearest codevector index for the target vector
	int getNearest(const std::vector<ldouble>& target){
		int k = target.size();
		int size = codebook.size();
		if(size == 0) return -1;

		ldouble minDistance = distance(target, codebook[0]); //assume codevector 0 to be closest
		int minIndex = 0;

		for(int i=1; i<size; i++){
			ldouble dist = distance(target, codebook[i]);
			if(dist < minDistance){
				minDistance = dist;
				minIndex = i;
			}
		}
		return minIndex;
	}
	//returns size
	int size(){
		return codebook.size();
	}
	//prints all the entries of the codebook
	void print(){
		int size=codebook.size();
		for(int i=0; i<size; i++){
			int k = codebook[i].size();
			std::cout << i << "-> ";
			for(int j=0; j<k; j++)
				std::cout << codebook[i][j] << " ";
			std::cout << "\n";
		}
		std::cout << "\n\n";
	}

	static Codebook readCodebook(std::string fileName){
		std::ifstream file(fileName);
	
		Codebook cb;
		if(file.is_open()){
			ldouble val;
			std::string line;		
			while(std::getline(file, line)){
				std::vector<ldouble> vector;
				std::stringstream ss(line);
				while(ss >> val)
					vector.push_back(val);
				cb.addEntry(vector);
			}
	
		}
		else{
			std::cout << fileName << "NOT OPENED\n";
		}
		return cb;
	}

};


#endif