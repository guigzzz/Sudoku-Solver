#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <time.h>

using std::endl;
using std::cout;
using std::cin;
using std::string;
using std::vector;
using std::ifstream;

void sudokusolver(vector<int>& linsud,vector<vector<vector<int>>>& possible_moves);
void extract_row(const vector<int>& vin, int& row, int *out);
void extract_col(const vector<int>& vin, int& col, int *out);
void extract_subsquare(const vector<int>& vin, int& row, int& col, int *out);
void coordtoidx(int& row,int& col,int& idx);
void copy(vector<int>& linsud,int& rowid,int *tmp);
bool sudoku_done(const vector<vector<int>>& linsud, int& game_done_index);
bool found_in_vector(const int *vin,int& item);
bool dead_end(const vector<vector<vector<int>>>& possible_moves);
bool need_guess(const vector<vector<vector<int>>>& possible_moves,vector<int>& guesses,int& row);
bool sudoku_check(const vector<int> sudoku);

int main(){
	cout << "Enter the name of the input file" << endl;
	ifstream infile;
	string infilename;
	cin >> infilename;
	infile.open(infilename.c_str());
	if(!infile.is_open()){
		cout << "could not open input file" << endl;
		exit(EXIT_FAILURE);
	}
	vector<int> linsud;
	int n;
	while(infile>>n){
		linsud.push_back(n);
	}
	
	clock_t t1,t2;
    	t1=clock();
	
	cout << "starting grid" << endl << endl;
		
	for(int i=0;i<9;i++){
		cout << endl;
		for(int j=0;j<9;j++){
			cout << linsud[9*i+j] << " ";
		}
	}
	
	cout << endl;
	
	vector<vector<int>> games;
	games.push_back(linsud);
	
	int index;
	int num_loops = 0;
	int max_concurrent_games = 0;
	
	while(!sudoku_done(games,index)){
		
	
		vector<vector<int>> new_games;
	
		for(int i = 0;i<games.size();i++){
			num_loops++;
		
			vector<vector<vector<int>>> possible_moves;
			vector<int> guesses;
			int row_guess;
		
			sudokusolver(games[i],possible_moves);
		
			if(need_guess(possible_moves,guesses,row_guess)){
				int number_to_guess = guesses[0];
			
				for(int j = 1;j<guesses.size();j++){
					//generate all new games with different guesses
					vector<int> tmpgame = games[i];
					tmpgame[row_guess*9+guesses[j]] = number_to_guess;
					new_games.push_back(tmpgame);
				}
			}
			else if(!dead_end(possible_moves)){
				new_games.push_back(games[i]);
			}
				
		}
		if(games.size()>max_concurrent_games){
			max_concurrent_games = games.size();
		}
		
		games = new_games;
		
	}
	
	
	if(sudoku_check(games[index])){
		cout << endl << "solution" << endl;
		for(int i=0;i<9;i++){
			cout << endl;
			for(int j=0;j<9;j++){
				cout << games[index][9*i+j] << " ";
			}
		}
		cout << endl << endl;
	}
	else{
		cout << "solution not found" << endl;
	}
	cout << endl << "stats:" << endl;
	cout << "max concurrent open games: " << max_concurrent_games << endl;
	cout << "number of game evaluations: " << num_loops << endl;
	t2=clock();
    	float diff ((float)t2-(float)t1);
    	cout << "time taken in seconds: " << diff/CLOCKS_PER_SEC << endl;
	infile.close();
	return 0;
}

void sudokusolver(vector<int>& linsud,vector<vector<vector<int>>>& possible_moves_high){
	//need to save possible moves in v of v of v
	//top level designates rows
	//second level vectors for all possible numbers in a certain row
	//last level vector of possible indices for the number
	int *coltmp = (int *)malloc(9*sizeof(int));
	int *rowtmp = (int *)malloc(9*sizeof(int));
	int *sstmp = (int *)malloc(9*sizeof(int));
	for(int i=0;i<9;i++){
		extract_row(linsud,i,rowtmp);
		//cout << "row " << i << endl;
		vector<vector<int>> possible_moves_mid;
		for(int k=1;k<=9;k++){
			vector<int> possible_moves_low;
			possible_moves_low.push_back(k);
			if(!found_in_vector(rowtmp,k)){
				for(int j=0;j<9;j++){
					if(rowtmp[j]==0){
						extract_col(linsud,j,coltmp);
						extract_subsquare(linsud,i,j,sstmp);
						if(!found_in_vector(coltmp,k) && !found_in_vector(sstmp,k)){
							possible_moves_low.push_back(j);
						}
					
					}
				}
				
				if(possible_moves_low.size() == 2){
					rowtmp[possible_moves_low[1]] = k;
				}
				possible_moves_mid.push_back(possible_moves_low);
			}
			
		}
		copy(linsud,i,rowtmp);
		possible_moves_high.push_back(possible_moves_mid);
	}
	free(coltmp);
	free(rowtmp);
	free(sstmp);
}

bool dead_end(const vector<vector<vector<int>>>& possible_moves){
	for(int rows = 0;rows<possible_moves.size();rows++){
		for(int possible_numbers = 0;possible_numbers<possible_moves[rows].size();possible_numbers++){
			if(possible_moves[rows][possible_numbers].size() == 1){
				return true;
			}
		}
	}
	return false;
}

bool need_guess(const vector<vector<vector<int>>>& possible_moves,vector<int>& guesses,int& row){
	int mini = 10;
	bool guess = true;
	for(int i = 0;i<possible_moves.size();i++){ //go thru rows of sudoku
		for(int j = 0;j<possible_moves[i].size();j++){ //go thru all possible numbers to add
			if(possible_moves[i][j].size() == 2){ //if a number is for certain no need to guess
				guess = false;
			}
			else if(possible_moves[i][j].size() < mini && guess){ //find number with minimum number of guesses to make
				guesses = possible_moves[i][j];
				mini = possible_moves[i][j].size();
				row = i;
			}
		}
	}
	return guess;
}

bool sudoku_done(const vector<vector<int>>& linsud, int& game_done_index){
	for(int i = 0;i<linsud.size();i++){
		bool curgamedone = true;
		for(int j = 0;j<linsud[i].size();j++){
			if(linsud[i][j] == 0){
				curgamedone = false;
			}
		}
		if(curgamedone){
			game_done_index = i;
			return true;
		}
	}
	return false;
}

bool found_in_vector(const int *vin,int& item){
	for(int i = 0;i<9;i++){
		if(vin[i] == item){
			return true;
		}
	}
	return false;
}

void copy(vector<int>& linsud,int& rowid,int *tmp){
	for(int i=0;i<9;i++){
		linsud[rowid*9+i]=tmp[i];
	}
}

void extract_row(const vector<int>& vin, int& row, int *out){
	int idx;
	for(int i=0;i<=8;i++){
		coordtoidx(row,i,idx);
		out[i] = vin[idx];
	}
}

void extract_col(const vector<int>& vin, int& col, int *out){
	int idx;
	for(int i=0;i<=8;i++){
		coordtoidx(i,col,idx);
		out[i] = vin[idx];
	}
}

void extract_subsquare(const vector<int>& vin, int& row, int& col, int *out){
	int idx;
	int r;
	int c;
	if(row<3){
		r=0;
	}
	else if(row >=3 && row<6){
		r=3;
	}
	else if(row>=6 && row<9){
		r=6;
	}
	if(col<3){
		c=0;
	}
	else if(col>=3 && col<6){
		c=3;
	}
	else if(col>= 6 && col<9){
		c=6;
	}
	for(int i=r;i<=r+2;i++){
		for(int j=c;j<=c+2;j++){
			coordtoidx(i,j,idx);
			out[(i-r)*3+j-c] = vin[idx];
		}
	}
}

void coordtoidx(int& row,int& col,int& idx){
	idx=(9*row)+col;
}

bool sudoku_check(const vector<int> sudoku){
	int *coltmp = (int *)malloc(9*sizeof(int));
	int *rowtmp = (int *)malloc(9*sizeof(int));
	int *sstmp = (int *)malloc(9*sizeof(int));
	for(int i = 0;i<9;i++){
		
		extract_row(sudoku,i,rowtmp);
		extract_col(sudoku,i,coltmp);
		
		for(int k=1;k<=9;k++){
			if(!found_in_vector(rowtmp,k) || !found_in_vector(coltmp,k)){
				free(coltmp);
				free(rowtmp);
				free(sstmp);
				return false;
			}
		}
		
	}
	
	for(int i = 0;i<9;i++){
		for(int j = 0;j<9;j++){
			for(int k=1;k<=9;k++){
				extract_subsquare(sudoku,i,j,sstmp);
				if(!found_in_vector(sstmp,k)){
					free(coltmp);
					free(rowtmp);
					free(sstmp);
					return false;
				}
			}
		}
	}
	free(coltmp);
	free(rowtmp);
	free(sstmp);
	return true;
}
