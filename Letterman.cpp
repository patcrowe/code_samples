/*
Patrick Crowe
<patcrowe@umich.edu>

Eecs281 Project 1
Letterman.cpp

This program runs through words in a dictionary from a starting word to an ending word.
To get there the program is allowed to make certain changes to the word specified by the 
command line arguments. 
Change- acceptable transition if one word can be become another by changing a single letter
Swap- acceptable transition if one word can become another by swapping two of the letters
Length- acceptable transition if one word can become another by adding or deleting a single letter
The program uses either a queue or a stack based on command line arguments
In complex input mode:
--If an (&) appears at the end of the word, then both the word and its reversal are generated
--If a set of characters appears inside square brackets ([...]), N words are generated with each character
  in the brackets being used once. ex: h[aio]t would produce hat, hit, hot in the dictionary
--If an exclamation point appears after two characters, then the original word and the word with the 
  two previous characters swapped are generated. ex: col!t would produce colt, clot
--​If a question mark appears after a character, then the original word and the word with 
  the one previous character doubled are generated. ex: ho?t would produce hot, hoot
*/

#include <string>
#include <deque>
#include <getopt.h>
#include <vector>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <iostream>

using namespace std;


class Policies {
public:
	Policies()
		:is_change(false), is_swap(false), is_length(false), is_stack(false), is_queue(false), output_mode("W"), begin(), end() {}

	void get_options(int argc, char** argv);

	string get_optarg_string() {
		if (optarg == nullptr || optarg[0] == '-') {
			cerr << "Invalid argument" << endl;
			exit(1);
		}
		else {
			return string(optarg);
		}
	}

	bool is_change;
	bool is_swap;
	bool is_length;
	bool is_stack; 
	bool is_queue;
	string output_mode;
	string begin;
	string end;
};

class Dictionary : public Policies {
public:

	struct Word {
		string word;
		Word* previous = nullptr;
		bool used = false;
		char modification = '\0';
		size_t mod_index = -1;
		char new_letter = '\0';
		size_t dict_index;
	};

	void read_data();
	void run();
	void do_change(Word* temp);
	void do_swap(Word* temp);
	void do_length(Word* temp);
	void display_output();

	vector<Word> dictionary;

	std::deque<Word*> deque;

	Word* current;
	Word* start;
	bool failed = false;
	bool success = false;
	int words_checked = 0;

};


int main(int argc, char** argv) {
	std::ios_base::sync_with_stdio(false);

	Dictionary dict1;

	dict1.get_options(argc, argv);

	dict1.read_data();

	dict1.run();

	return 0;
}



  // Read in the words to the dictionary
void Dictionary::read_data() {
	char dictionary_type;
	int lines; //number of words in the dictionary
	bool start_in_dict = false; //checking if the starting word is in the dictionary
	bool end_in_dict = false; //checking if the ending word is in the dictionary

	cin >> dictionary_type >> ws;
	
	if (dictionary_type == 'S') {  //Simple reading
		cin >> lines >> ws;
		dictionary.reserve(lines);
		string line;
		while (getline(cin, line)) {
			istringstream iss(line);
			Word temp;
			iss >> temp.word;
			if (temp.word[0] != '/') {
				if (!is_length) {
					if (temp.word.size() == end.size()) {
						temp.dict_index = dictionary.size();
						dictionary.push_back(temp);
					}
				}
				else {
					temp.dict_index = dictionary.size();
					dictionary.push_back(temp);
				}
			}
		}
	}
	else if (dictionary_type == 'C') {  //Complex reading
		cin >> lines >> ws;
		string line;
		while (getline(cin, line)) {
			istringstream iss(line);
			Word temp;
			iss >> temp.word;
			if (temp.word[0] != '/' && temp.word[1] != '/') {
				if (temp.word[temp.word.size() - 1] == '&') {
					temp.word.erase(temp.word.size() - 1);
					temp.dict_index = dictionary.size();
					dictionary.push_back(temp);
					reverse(temp.word.begin(), temp.word.end());
					temp.dict_index = dictionary.size();
					dictionary.push_back(temp);
				}
				else if (temp.word.find('?') != string::npos) {
					size_t double_index = temp.word.find('?') - 1;
					char double_letter = temp.word[double_index];
					temp.word.erase(temp.word.begin() + double_index + 1);
					temp.dict_index = dictionary.size();
					dictionary.push_back(temp);
					temp.word.insert(temp.word.begin() + double_index, double_letter);
					temp.dict_index = dictionary.size();
					dictionary.push_back(temp);
				}
				else if (temp.word.find('!') != string::npos) {
					size_t switch_index = temp.word.find('!') - 1;
					temp.word.erase(temp.word.begin() + switch_index + 1);
					temp.dict_index = dictionary.size();
					dictionary.push_back(temp);
					swap(temp.word[switch_index], temp.word[switch_index - 1]);
					temp.dict_index = dictionary.size();
					dictionary.push_back(temp);
				}
				else if (temp.word.find('[') != string::npos) {
					size_t first_bracket = temp.word.find('[');
					size_t second_bracket = temp.word.find(']');
					if (second_bracket - first_bracket == 1) {
						temp.word.erase(second_bracket);
						temp.word.erase(first_bracket);
						temp.dict_index = dictionary.size();
						dictionary.push_back(temp);
					}
					else {
						Word new_temp;
						for (size_t i = first_bracket + 1; i < second_bracket; i++) {
							new_temp.word = temp.word;
							char current_char = temp.word[i];
							string current_letter;
							current_letter.push_back(current_char);
							new_temp.word.replace(new_temp.word.begin() + first_bracket, new_temp.word.begin() + second_bracket + 1, current_letter);
							new_temp.dict_index = dictionary.size();
							dictionary.push_back(new_temp);
						}
					}
				}
				else {
					temp.dict_index = dictionary.size();
					dictionary.push_back(temp);
				}
			}
		}
	}


	for (size_t i = 0; i < dictionary.size(); i++) {
		if (dictionary[i].word == begin) {
			start_in_dict = true;
		}
		else if (dictionary[i].word == end) {
			end_in_dict = true;
		}
	}
	if (!start_in_dict || !end_in_dict) {
		cerr << "Both the beginning and ending words must exist in the dictionary." << endl;
		exit(1);
	}
}


void Dictionary::run() {

	for (size_t i = 0; i < dictionary.size(); i++) {
		if (dictionary[i].word == begin) {
			start = &dictionary[i];
			(*start).used = true;
			deque.push_back(start);
			words_checked++;
			break;
		}
	}
	
	while (!failed || !success) {
		if (deque.empty()) {
			failed = true;
			break;
		}
		else if (is_stack) {
			current = deque.back();
			deque.pop_back();
		}
		else {
			current = deque.front();
			deque.pop_front();
		}
		if ((*current).word == end) {
			success = true;
			break;
		}
		for (size_t i = 0; i < dictionary.size(); i++) {
			Word* temp;
			temp = &dictionary[i];
			
			if (!(*temp).used && (*temp).word != (*current).word) {
				if (is_change && (*temp).word.size() == (*current).word.size()) do_change(temp);
				if (is_swap && (*temp).word.size() == (*current).word.size() 
					&& (*temp).word.size() > 1) do_swap(temp);
				if (is_length && (((*temp).word.size() - (*current).word.size() == 1) 
					|| ((*current).word.size() - (*temp).word.size() == 1))) do_length(temp);
			}
			if (success) break;
		}
		if (deque.empty()) failed = true;
	}

	if (failed) {
		cout << "No solution, " << words_checked << " words checked.";
	}
	else {
		display_output();
	}
}


void Dictionary::do_change(Word* temp) {
	size_t diff_index = -1;
	int difference = 0;
	for (size_t j = 0; j < (*temp).word.size(); j++) {
		if ((*temp).word[j] != (*current).word[j]) {
			difference++;
			diff_index = j;
		}
	}
	if (difference == 1) {
		(*temp).modification = 'c';
		(*temp).mod_index = diff_index;
		(*temp).used = true;
		(*temp).new_letter = (*temp).word[diff_index];
		words_checked++;
		(*temp).previous = current;
		deque.push_back(temp);
		if ((*temp).word == end) success = true;
	}
}

void Dictionary::do_swap(Word* temp) {
	size_t diff_index1 = -1;
	size_t diff_index2 = -1;
	int difference = 0;
	for (size_t j = 0; j < (*temp).word.size(); j++) {
		if ((*temp).word[j] != (*current).word[j]) {
			difference++;
			diff_index1 = j;
			break;
		}
	}
	for (size_t j = diff_index1 + 1; j < (*temp).word.size(); j++) {
		if ((*temp).word[j] !=(*current).word[j]) {
			difference++;
			diff_index2 = j;
		}
	}
	if (difference == 2 && (diff_index2 - diff_index1 == 1)) {
		if ((*temp).word[diff_index1] == (*current).word[diff_index2] 
			&& (*temp).word[diff_index2] == (*current).word[diff_index1]) {
			(*temp).modification = 's';
			(*temp).mod_index = diff_index1;
			(*temp).used = true;
			words_checked++;
			(*temp).previous = current;
			deque.push_back(temp);
			if ((*temp).word == end) success = true;
		}
	}
}

void Dictionary::do_length(Word* temp) {
	size_t diff_index = -1;
	bool possible = true;
	size_t buff_start = 0;
	if ((*temp).word.size() < (*current).word.size()) {
		if ((*temp).word.size() == 1) {
			if ((*temp).word[0] == (*current).word[0]) {
				possible = true;
				diff_index = 1;
			}
			else if ((*temp).word[0] == (*current).word[1]) {
				possible = true;
				diff_index = 0;
			}
			else {
				possible = false;
			}
		}
		else {
			if ((*temp).word[0] != (*current).word[0]) {
				for (size_t j = 0; j < (*current).word.size(); j++) {
					if ((*temp).word[j] == (*current).word[j + 1]) {
						possible = true;
					}
					else {
						possible = false;
						break;
					}
				}
				if (possible) {
					diff_index = 0;
				}
			}
			else {
				for (size_t j = 1; j < (*temp).word.size(); j++) {
					if ((*temp).word[j] == (*current).word[j]) {
						possible = true;
					}
					else {
						buff_start = j;
						break;
					}
				}
				if (buff_start == 0) {
					diff_index = (*temp).word.size();
				}
				else {
					diff_index = buff_start;
					for (size_t j = buff_start; j < (*temp).word.size(); j++) {
						if ((*temp).word[j] == (*current).word[j + 1]) {
							possible = true;
						}
						else {
							possible = false;
							break;
						}
					}
				}
			}
		}

		if (possible) {
			(*temp).modification = 'd';
			(*temp).mod_index = diff_index;
			(*temp).used = true;
			words_checked++;
			(*temp).previous = current;
			deque.push_back(temp);
			if ((*temp).word == end) success = true;
		}
	}
	else {
		char diff_char = '\0';
		if ((*temp).word.size() == 2) {
			if ((*temp).word[0] == (*current).word[0]) {
				possible = true;
				diff_index = 1;
				diff_char = (*temp).word[1];
			}
			else if ((*temp).word[1] == (*current).word[0]) {
				possible = true;
				diff_index = 0;
				diff_char = (*temp).word[0];
			}
			else {
				possible = false;
			}
		}
		else {
			if ((*temp).word[0] != (*current).word[0]) {
				for (size_t j = 0; j < (*current).word.size(); j++) {
					if ((*temp).word[j + 1] == (*current).word[j]) {
						possible = true;
					}
					else {
						possible = false;
						break;
					}
				}
				if (possible) {
					diff_char = (*temp).word[0];
					diff_index = 0;
				}
			}
			else {
				for (size_t j = 1; j < (*current).word.size(); j++) {
					if ((*temp).word[j] == (*current).word[j]) {
						possible = true;
					}
					else {
						buff_start = j;
						break;
					}
				}
				if (buff_start == 0) {
					diff_index = (*current).word.size();
					diff_char = (*temp).word[diff_index];
				}
				else {
					diff_index = buff_start;
					diff_char = (*temp).word[diff_index];
					for (size_t j = buff_start; j < (*current).word.size(); j++) {
						if ((*temp).word[j + 1] == (*current).word[j]) {
							possible = true;
						}
						else {
							possible = false;
							break;
						}
					}
				}
			}
		}
		if (possible) {
			(*temp).modification = 'i';
			(*temp).mod_index = diff_index;
			(*temp).used = true;
			(*temp).new_letter = diff_char;
			words_checked++;
			(*temp).previous = current;
			deque.push_back(temp);
			if ((*temp).word == end) success = true;
		}
	}
}

void Dictionary::display_output() {
	Word* temp = current;
	vector<Word> path;
	path.reserve(words_checked);
	while ((*temp).previous != nullptr) {
		path.push_back(*temp);
		temp = (*temp).previous;
	}
	path.push_back(*temp);
	reverse(path.begin(), path.end());

	cout << "Words in morph: " << path.size();
	if (output_mode == "W") {
		for (size_t i = 0; i < path.size(); i++) {
			cout << '\n' << path[i].word;
		}
	}
	else {
		cout << '\n' << path.front().word;
		for (size_t i = 1; i < path.size(); i++) {
			cout << '\n' << path[i].modification << "," << path[i].mod_index;
			if (path[i].modification == 'c' || path[i].modification == 'i') {
				cout << "," << path[i].new_letter;
			}
		}
	}
}



// Read and process command line options.
void Policies::get_options(int argc, char** argv) {
	int option_index = 0, option = 0; 

	// Don't display getopt error messages about options
	opterr = false;


	// use getopt to find command line options
	struct option longOpts[] = { { "output", required_argument, nullptr, 'o' },
								 { "stack", no_argument, nullptr, 's' },
								 { "queue", no_argument, nullptr, 'q' },
								 { "change", no_argument, nullptr, 'c' },
								 { "swap", no_argument, nullptr, 'p' },
								 { "length", no_argument, nullptr, 'l'},
								 { "begin", required_argument, nullptr, 'b'},
								 { "end", required_argument, nullptr, 'e'},
								 { "help", no_argument, nullptr, 'h'},
								 { nullptr, 0, nullptr, '\0' }
	};

	while ((option = getopt_long(argc, argv, "o:sqcplb:e:h", longOpts, &option_index)) != -1) {
		switch (option) {
		case 'o':
			output_mode = get_optarg_string();
			break;

		case 'b':
			if (begin == "") {
				begin = get_optarg_string();
			}
			else {
				cerr << "Beginning word already set." << endl;
				exit(1);
			}
			break;

		case 'e':
			if (end == "") {
				end = get_optarg_string();
			}
			else {
				cerr << "Ending word already set." << endl;
				exit(1);
			}
			break;

		case 's':
			if (is_stack) {
				cerr << "Stack already set" << endl;
				exit(1);
			}
			is_stack = true;
			break;

		case 'q':
			if (is_queue) {
				cerr << "Queue already set" << endl;
				exit(1);
			}
			is_queue = true;
			break;

		case 'c':
			if (is_change) {
				cerr << "Change already set" << endl;
				exit(1);
			}
			is_change = true;
			break;

		case 'p':
			if (is_swap) {
				cerr << "Swap already set" << endl;
				exit(1);
			}
			is_swap = true;
			break;

		case 'l':
			if (is_length) {
				cerr << "Length already set" << endl;
				exit(1);
			}
			is_length = true;
			break;

		case 'h':
			cout << "This program reads an input file that contains a dictionary.\n"
				<< "It then runs through the words in the dictionary\n"
				<< "to get from a starting word to an ending word.\n"
				<< "It progresses from word to word depending on the\n"
				<< "allowed types of morphs given on the command line:\n"
				<< "Change allows the program to change one letter of a word\n"
				<< "Swap allows the program to switch two adjecent letters in a word\n"
				<< "length allows the program to add or remove one letter from a word\n"
				<< "There are two routing schemes the program can take, stack or queue\n"
				<< "There are two output modes for the program:\n"
				<< "Word mode prints each word in the progression from the start to the end\n"
				<< "Morph mode prints the types of morphs used to get to each next word\n"
				<< "Usage: \'./letterman\n\t[--change | -c]\n"
				<< "\t[--swap | -p]\n"
				<< "\t[--length | -l]\n"
				<< "\t[--output | -o] <mode of output W or M>\n"
				<< "\t[--stack | -s]\n"
				<< "\t[--queue | -q]\n"
				<< "\t[--begin | -b] <beginning word>\n"
				<< "\t[--end | -e] <ending word>\n"
				<< "\t[--help | -h]\n"
				<< "\t< <Dictionary File>\'" << endl;
			exit(0);
		}
	}

	if (output_mode != "M" && output_mode != "W") {
		cerr << "Invalid argument for Output Mode. Valid arguments are 'M' and 'W'." << endl;
		exit(1);
	}

	if ((is_stack && is_queue) || (!is_stack && !is_queue)) {
		cerr << "Must specify exactly one routing scheme, stack or queue." << endl;
		exit(1);
	}

	if (!is_change && !is_length && !is_swap) {
		cerr << "Must specify allowed types of morphs. Valid types are\n"
			<< "change, swap, and length." << endl;
		exit(1);
	}

	if ((begin.length() != end.length()) && !is_length) {
		cerr << "Must allow length type morph if beginning and ending\n"
			<< "words are of different lengths." << endl;
		exit(1);
	}

	if (begin.empty() || end.empty()) {
		cerr << "Must provide both a starting word and an ending word." << endl;
		exit(1);
	}
}
