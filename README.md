HEES
====

0 Download and Compile
------------------------

To run the simulator, you will need the simulator itself,
the sundials library and the boost library.

### 0.1 Download the simulator

### 0.2 Download and install the sundials library

1. Download the library from: [sundials website](http://computation.llnl.gov/casc/sundials/download/download.php).
   You only need the KINSOL package though.

2. untar the package, and go into the directory:

   ```
   tar -xzvf kinsol-2.7.0.tar.gz
   cd kinsol-2.7.0
   ```

   I would recommand you to read section B.1, B.2 and B.4 of the INSTALL\_NOTES,
   but if you don't want to, then you can just do:

   ```bash
   # Assume I want to install them to ~/usr/sundials directory
   ./configure --prefix=$HOME/usr/sundials --enable-examples
   make
   make install
   ```

### 0.3 Download and install the boost library
What we really need from boost library is just the program\_options.

#### 0.3.1 Install on Ubuntu

1. To find the this library in your Ubuntu release:

  ```
  sudo apt-cache search libboost-program-options\*
  ```
2. To install them, my Ubuntu comes with 1.55 version, so I just do

  ```
  sudo apt-get install libboost1.55-dev libboost-program-options1.55-dev libboost-program-options1.55.0
  ```

  Note that you will need the libboost1.55-dev for the header file "boost/program\_options.hpp"
3. Finally, you can verify your installation by using the command:
  
  ```
  dpkg-query --listfiles libboost1.55-dev

  dpkg-query --listfiles libboost-program-options1.55.0
  ```

  To check if the files have been installed correctly or just try to compile them.

#### 0.3.2 Install on Redhat

#### 0.3.3 Install from scratch

### 0.4 Compile the simulatro
