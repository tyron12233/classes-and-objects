#include <functional>
#include <iomanip>
#include <iostream>
#include <regex>
#include <vector>

using namespace std;

/**
 * Clears the console screen depending on the operating system.
 * Needed cause I use linux and the person who will run this code may use windows.
 */
void clearScreen() {
#ifdef WINDOWS
    std::system("cls");
#else
    std::system("clear");
#endif
}

/**
 *  A helper function to get a validated string from the user.
 *
 *  The user is prompted with the given message, and the input is read from the console.
 */
string getValidatedString(const string &prompt, const regex &regex) {
    string input;
    cout << prompt;

    getline(cin, input);

    if (regex_match(input, regex)) {
        return input;
    }

    clearScreen();
    cout << "Invalid input. Please try again." << endl;
    return getValidatedString(prompt, regex);
}

/**
 * Models a book
 */
class Book {
    string title;
    string author;
    string year;

public:
    Book(const string &title, const string &author, const string &year) {
        this->title = title;
        this->author = author;
        this->year = year;
    }

    static Book getBookFromInput() {
        cout << "Enter book details: " << endl << endl;
        const string title = getValidatedString("Enter title: ", regex("^[a-zA-Z0-9 ]+$"));
        const string author = getValidatedString("Enter author: ", regex("^[a-zA-Z0-9 ]+$"));
        const string year = getValidatedString("Enter year: ", regex("^[0-9]{4}$"));

        return Book{title, author, year};
    }

    string getTitle() {
        return title;
    }

    string getAuthor() {
        return author;
    }

    string getYear() {
        return year;
    }
};

/**
 * Models an action that can be perfomed in the library.
 * E.g Add a book, search a book, display books.
 */
class LibraryAction {
    string title;

    // what to do when this menu item is selected, the given function
    // should take a vector of books as an argument
    function<void(vector<Book>)> action;

public:
    LibraryAction(const string &title, const function<void(vector<Book>)> &action) {
        this->title = title;
        this->action = action;
    }

    string getTitle() {
        return title;
    }

    function<void(vector<Book>)> getAction() {
        return action;
    }
};


/**
 * Responsible for managing the library of books,
 * including adding, searching, and displaying books.
 *
 * Books are stored internally in a vector.
 */
class Library {
    // the internal list of books
    vector<Book> books;

    // actions that can be performed in this library
    vector<LibraryAction> actions;

public:
    Library() {
        // initialize the library with no books
        this->books = {};
        this->actions = {};
    }

    // runs the library, displaying the menu and handling user input
    // until the user chooses to exit
    void run() {
        // flag to keep track of whether the library is running,
        // when this is set to false, the library will exit
        bool running = true;

        // this is defined here because this should be the last action
        // in the list of actions
        LibraryAction exitAction("Exit", [&running](const vector<Book> & /*books*/) {
            clearScreen();
            cout << "Thank you for using the library!" << endl;
            running = false;
        });

        // display the menu and handle user input until the user chooses to exit
        function displayMenu = [this, &exitAction]() {
            clearScreen();
            cout << "Welcome to the library!" << endl << endl;
            cout << "Please choose an action:" << endl;

            const size_t maxIndex = actions.size();
            for (size_t i = 0; i < maxIndex; i++) {
                cout << i + 1 << ". " << actions[i].getTitle() << endl;
            }

            cout << (maxIndex + 1) << ". " << exitAction.getTitle() << endl;
            cout << endl;
        };

        while (true) {
            clearScreen();
            displayMenu();

            // user input validation
            int choice = 0;
            while (true) {
                cout << "Enter your choice: ";
                if (!(cin >> choice)) {
                    clearScreen();
                    displayMenu();

                    cout << "Invalid input. Please try again." << endl;
                    cin.clear();
                    cin.ignore();
                    continue;
                }

                if (choice < 1 || choice > actions.size() + 1) {
                    clearScreen();
                    displayMenu();

                    cout << "Invalid input. Please try again." << endl;
                    continue;
                }

                cin.ignore();
                break;
            }

            // input must already be validated at this point
            // but since our list is 0-indexed, we need to subtract 1
            if (choice == actions.size() + 1) {
                exitAction.getAction()(books);
                break;
            }


            clearScreen();
            actions[choice - 1].getAction()(books);
        }
    }

    /**
     * Adds an action to this library.
     */
    void addAction(const string &name, const function<void(vector<Book>)> &action) {
        const auto libraryAction = LibraryAction(name, action);
        actions.push_back(libraryAction);
    }

    /**
     * Finds a book in the library by title, author, and year.
     * These fields must match exactly for a book to be found.
     *
     * warning: this method returns a pointer to a book, which may be null if no book is found.
     */
    Book *findBook(const string &title) {
        for (auto &book: books) {
            if (book.getTitle() == title) {
                return &book;
            }
        }

        return nullptr;
    }

    /**
     * Adds a book to the library.
     */
    void addBook(const Book &book) {
        books.push_back(book);
    }
};


int main() {
    constexpr int nameWidth = 20;
    constexpr int authorWidth = 20;
    constexpr int yearWidth = 10;

    // initialize the library class here
    Library library;

    // here we add the actions that can be performed in the library
    // this way, we can easily add more actions in the future

    library.addAction("Add a book", [&library](const vector<Book> & /*books*/) {
        const auto book = Book::getBookFromInput();
        library.addBook(book);
    });

    library.addAction("Search book", [&library](const vector<Book> & /*books*/) {
        clearScreen();

        string title = getValidatedString("Enter book title: ", regex("^[a-zA-Z0-9 ]+$"));
        Book *foundBook = library.findBook(title);

        bool bookExists = foundBook != nullptr;

        clearScreen();
        if (bookExists) {
            // dereference the pointer to get the actual book
            auto book = *foundBook;

            cout << "Book found." << endl << endl;
            cout << "┌─────────────────────┬─────────────────────┬───────────┐" << endl;
            cout << "│ Title               │ Author              │ Year      │" << endl;
            cout << "├─────────────────────┼─────────────────────┼───────────┤" << endl;
            cout << "│ " << left << setw(nameWidth) << book.getTitle() << "│ " << left << setw(authorWidth) << book.
                    getAuthor() << "│ " << left << setw(yearWidth) << book.getYear() << "│" << endl;
            cout << "└─────────────────────┴─────────────────────┴───────────┘" << endl;
        } else {
            cout << "Book not found." << endl;
        }

        cout << "Press enter to continue..." << endl;
        cin.get();
    });

    library.addAction("Display books", [](const vector<Book> &books) {
        if (books.empty()) {
            cout << "┌───────────────────────────────────────────────────────┐" << endl;
            cout << "│                  No books to display                  │" << endl;
            cout << "└───────────────────────────────────────────────────────┘" << endl;
        } else {
            cout << "┌─────────────────────┬─────────────────────┬───────────┐" << endl;
            cout << "│ Title               │ Author              │ Year      │" << endl;
            cout << "├─────────────────────┼─────────────────────┼───────────┤" << endl;
            for (auto book: books) {
                cout << "│ " << left << setw(nameWidth) << book.getTitle() << "│ " << left << setw(authorWidth) << book.
                        getAuthor() << "│ " << left << setw(yearWidth) << book.getYear() << "│" << endl;
            }
            cout << "└─────────────────────┴─────────────────────┴───────────┘" << endl;
        }


        // press enter to continue
        cout << "Press enter to continue..." << endl;
        cin.get();
    });

    // this code will block until the user chooses to exit the library
    library.run();
}
