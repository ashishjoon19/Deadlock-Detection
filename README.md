Predict, Prevent and Proceed: Deadlock Handling
This project is a Qt-based C++ application designed to demonstrate deadlock handling in a multi-transaction environment. It provides a graphical user interface (GUI) to manage transactions and data items, visualize the Wait-For Graph, detect deadlocks, and recover from them. The project includes features for deadlock prevention and a user-friendly interface with recent enhancements.
Features

Transaction and Data Management: Add transactions and data items dynamically.
Lock Operations: Request and release locks on data items for specific transactions.
Deadlock Detection: Identify deadlocks using the Wait-For Graph and display cycles.
Deadlock Recovery: Resolve deadlocks by terminating a transaction in the cycle.
Deadlock Prevention: Option to enable prevention by enforcing lock ordering.
Enhanced Wait-For Graph:
Full-screen view option for better visibility.
Curved, directed lines with arrows to show dependencies.
Thicker lines (weight 4) for improved visibility.
Bidirectional edges between transactions are drawn with one line above and one below to avoid overlap.


User-Friendly GUI:
Tooltips on buttons explaining their functions.
Clear labels for input fields (TID and DID).
User guide messages:
"To add transaction, click on Add Transaction button"
"To add data, click on Add Data Item button"


Stretchable tables for better readability.



Requirements

Qt Framework: Version 6.x (tested with Qt 6.9.0).
C++ Compiler: A compiler supporting C++11 or later (e.g., MinGW for Windows).
Operating System: Windows, Linux, or macOS (developed on Windows).
Qt Creator (optional): For easier project management and building.
Git: Required to clone the repository.

Clone the Repository
To get a copy of this project on your local machine, you need to clone the repository using Git. Follow these steps:

Install Git:

If you don’t have Git installed, download and install it from git-scm.com.


Clone the Repository:

Open a terminal or command prompt.
Run the following command, replacing [REPOSITORY_URL] with the URL of your repository (e.g., from GitHub, GitLab, or Bitbucket):git clone [REPOSITORY_URL]

Example:git clone https://github.com/yourusername/deadlock-handling.git


This will create a directory named after the repository (e.g., deadlock-handling) containing all project files.


Navigate to the Project Directory:

Change into the project directory:cd deadlock-handling





Setup Instructions

Install Qt:

Download and install Qt from qt.io.
Ensure you select a version compatible with your compiler (e.g., Qt 6.9.0 with MinGW).
Set up Qt Creator if you prefer using an IDE.


Open the Project in Qt Creator:

Launch Qt Creator.
Open the project by selecting the .pro file.
Configure the project with a kit (e.g., Desktop Qt 6.9.0 MinGW 64-bit).


Build the Project:

Click "Build" in Qt Creator (or run qmake followed by make in the terminal).
Ensure there are no compilation errors. If you encounter issues with QHeaderView, verify that all necessary Qt modules are included in your .pro file (see below).


Qt Project File Configuration:Ensure your .pro file includes the required modules:
QT += core gui widgets
CONFIG += c++11


Run the Application:

Click "Run" in Qt Creator (or execute the generated binary).



Usage Instructions

Launch the Application:

Run the application to open the main window titled "Predict, Prevent and Proceed: Deadlock Handling".


User Guide:

Follow the on-screen user guide messages:
"To add transaction, click on Add Transaction button"
"To add data, click on Add Data Item button"




Add Transactions and Data Items:

Click "Add Transaction" to create a new transaction (e.g., T0, T1).
Click "Add Data Item" to create a new data item (e.g., D0, D1).


Request and Release Locks:

Enter a Transaction ID (TID) and Data Item ID (DID) in the input fields.
Click "Request Lock" to have the transaction request a lock on the data item.
Click "Release Lock" to release a lock held by the transaction on the data item.


Detect and Recover from Deadlocks:

Click "Detect Deadlock" to check for cycles in the Wait-For Graph.
If a deadlock is detected, click "Recover" to terminate a transaction and resolve the deadlock.


Enable Deadlock Prevention:

Check the "Enable Prevention" checkbox to enforce lock ordering and prevent deadlocks.


View the Wait-For Graph:

The graph updates automatically after each operation.
Click "View Graph Full Screen" to see the graph in a larger view.
Edges are drawn with arrows, and bidirectional edges between transactions curve above and below to avoid overlap.



Testing
To verify the application works as expected, try this test case:

Add two transactions (T0 and T1).
Add two data items (D0 and D1).
Request lock: T0 on D0.
Request lock: T1 on D1.
Request lock: T0 on D1 (T0 waits for T1).
Request lock: T1 on D0 (T1 waits for T0, creating a cycle).
Click "Detect Deadlock" (should detect T0 → T1 → T0).
Observe the Wait-For Graph: edges between T0 and T1 should have one curve above and one below.
Click "Recover" to resolve the deadlock.

Future Improvements

Add support for different lock types (e.g., shared vs. exclusive locks).
Implement a history log to undo/redo operations.
Enhance the graph with zoom functionality.
Add color coding to highlight deadlock cycles in the graph.
