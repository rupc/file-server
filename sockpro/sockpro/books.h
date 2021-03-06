#include <iostream>
#include <string>
#include <list>
#include <algorithm>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <string.h>
using namespace std;

typedef struct {
	char		title[40];
	char		writer[20];
	char		publisher[20];
	unsigned	price;
	unsigned	department;
	float		value;

}book;

ostream& operator << (ostream &out, book &bk) {
	out << bk.title << " " << bk.writer << " " << bk.publisher << " " << bk.price << " " << bk.department << " " << bk.value << endl;
	return out;
}

istream& operator >> (istream &in, book &bk) {
	string t_title, t_writer, t_pub;
	unsigned t_price, dept, val;

	in >> t_title >> t_writer >> t_pub >> t_price >> dept >> val;

	strcpy(bk.title, t_title.c_str());
	strcpy(bk.writer, t_writer.c_str());
	strcpy(bk.publisher, t_pub.c_str());
	bk.price = t_price;
	bk.department = dept;
	bk.value = val;
	return in;
}

bool operator == (book b1, book b2) {
	if(strcmp(b1.title, b2.title) == 0 )
		return true;
	else return false;
}

class manage_book {
private:
	list<book> rec_list;

public:
	manage_book();
	void initialize_book_list_server_have(string &filename);
	void list_all_book(ostream &out);
	book find_one(book bk);
	void del_book(book bk);
	void add_book(book bk);
	unsigned num_of_books();
};

void manage_book::initialize_book_list_server_have(string &filename) {
	ifstream in(filename.c_str(), ios::in);

	while(!in.eof()) {
		book tmp;
		in >> tmp;
		rec_list.push_back(tmp);
	}
}

void manage_book::list_all_book(ostream &out) {
	list<book>::iterator b_iter = rec_list.begin();
	for(; b_iter != rec_list.end(); ++b_iter)
		out << *b_iter << endl;
}

void manage_book::add_book(book bk) {
	list<book>::iterator b_iter = rec_list.begin();
	if( find(rec_list.begin(), rec_list.end(), bk) == rec_list.end()) rec_list.push_back(bk);
}

void manage_book::del_book(book bk) {
	list<book>::iterator b_iter = find(rec_list.begin(), rec_list.end(), bk);
	if(b_iter != rec_list.end())
		rec_list.remove(bk);
}

book manage_book::find_one(book bk) {
	book tmp;
	list<book>::iterator b_iter = find(rec_list.begin(), rec_list.end(), bk);
	if(b_iter == rec_list.end()) {
		strcpy(tmp.title, "NONE");
		return tmp;
	} else
		return *b_iter;
}
unsigned manage_book::num_of_books() {
	return rec_list.size();
}

