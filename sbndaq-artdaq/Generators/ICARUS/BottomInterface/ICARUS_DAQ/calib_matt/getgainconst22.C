//#include <mysql/mysql.h>


/*void getgainconst(const char* onlinetable, const int usbnum, int pmtboardnumber[100], float gainused[100][64], int *numberofboardused, bool debug)   //input used to be pmtserialnumber instead of PMT_board_serial
{
        

  

//This function retrieves 64 gain constants used to take a run looking at the info stored into MySQL, 
//unders the following conditions:
//- Date (yyyymmdd);
//- Sub Run number;
//- PMT serial number (input without PA example = 4665);
//- HV used to take data (example = 800);

//- Function will generate warning if somenthing will go wrong -





	
	char sql[1000];
	//const char* servername = "mysql://riverside.nevis.columbia.edu:3306/doublechooz_ov_far"; //specify the mysql address
	//const char* username   = "dcdaq";				    			 // username
	//const char* userpass   = "doublechooz";				      		 // password
	const char* servername = "grape";				 			 // specify the mysql address
	const char* username   = "dcndaq";				    			 // username
	const char* userpass   = "137316neutron";				      		 // password
	int nfields;
	
   // open connection to MySQL server on localhost only for flag >0;
//	try{
        MYSQL mysql;
        MYSQL_RES *result;
        MYSQL_ROW sql_row;
        int res;


        mysql_init(&mysql);          //initial mysql structure

        if(mysql_real_connect(&mysql, servername, username, userpass, onlinetable,0,NULL,0)){

 	  cout << "Connected to MySQL database\n";  	  
	      
	   sprintf(sql, "SELECT board_number, gain1, gain2, gain3, gain4, gain5, gain6, gain7, gain8, gain9, gain10, gain11, gain12, gain13, gain14, gain15, gain16, gain17, gain18, gain19, gain20, gain21, gain22, gain23, gain24, gain25, gain26, gain27, gain28, gain29, gain30, gain31, gain32, gain33, gain34, gain35, gain36, gain37, gain38, gain39, gain40, gain41, gain42, gain43, gain44, gain45, gain46, gain47, gain48, gain49, gain50, gain51, gain52, gain53, gain54, gain55, gain56, gain57, gain58, gain59, gain60, gain61, gain62, gain63, gain64 FROM online_robeson_all WHERE USB_serial = %d AND (board_number = %d OR board_number = %d OR board_number = %d OR board_number = %d)",usbnum, pmtboardnumber[1], pmtboardnumber[2], pmtboardnumber[3], pmtboardnumber[4]);


	   res = mysql_query(&mysql, sql);       //SQL RESULT = SQL SERVER -> QUERY(sq1)

	   if(!res){
	     result = mysql_store_result(&mysql);
	     if(result){
	       *numberofboardused = mysql_affected_rows(&mysql);   //int nrows = SQL RESULT -> GetRowCount()

               if(debug) cout << "Number of board in table=" << *numberofboardused << endl;
 
	       if (*numberofboardused < 2)printf("\nWarning Got %d rows in result\n", *numberofboardused);

	       nfields = mysql_field_count(&mysql);

	       if (nfields != 65) printf("\nWarning Got %d fields in result\n", nfields);
	   
		  for (int i = 0; i < *numberofboardused; i++) {     //loop through the number of fields in SQL

		    sql_row = mysql_fetch_row(result);

                    pmtboardnumber[i+1] = atoi(sql_row[0]); 

		    for (int j = 1; j < nfields; j++) {     //loop through the number of fields in SQL
		      
		      gainused[pmtboardnumber[i+1]][j-1] = atof(sql_row[j]);	  //16.is needed because I am going to multiply the adc
		      // note, atof() converts the jth field of the row string to double
		      
		      if(debug) printf("Boardnumber = %d, --PMT channel=%d--gain constant applied=%f\n", pmtboardnumber[i+1], j, gainused[pmtboardnumber[i+1]][j-1]);
		    }
		  }
	     } //end if(result)
	   } // end if(!res)
	   else{
	     cout << "sql query failed\n";
	   }
	}
	else{
	  cout << "MySQL connection failed. Using hardcoded values.\n";
	  *numberofboardused = 4;
	  pmtboardnumber[1]=3;
	  pmtboardnumber[2]=7;
	  pmtboardnumber[3]=1;
	  pmtboardnumber[4]=2;
	}
      //}
      //catch(...){}
}
*/

void getgainfcl(string fclfilename, int pmtboardnumber[100], float gainused[100][64], int *numberofboardused){

    vector <vector<string>> output; 
    ifstream file (fclfilename);
    int lineNum = 1;   //Line tracker
    bool keepReading = true; //Keeps track of when to stop reading.
    string line; 
    if (file.is_open()){
        while(getline(file,line)){
            if (line[line.length()-1] == ']' && line[line.length()-2] == ']'){ keepReading = false; /*cout<<"set to false at line "<<lineNum<<'\n';*/}
            if (line[0] == ']'){ keepReading = false; /*cout<<"set to false at line "<<lineNum<<'\n';*/}
            if (line[0] == '#'){ keepReading = false; /*cout<<"set to false at line "<<lineNum<<'\n';*/}
            if (line[0] == '[' && line.length()>2){ keepReading = true;  /*cout<<"set to true at line "<<lineNum<<'\n'*/;}
                if(keepReading){                        
                    vector<string> v;
                    stringstream ss(line);

                    while(ss.good()){
                        string sub;
                        getline(ss, sub, ',');
                        if(!sub.empty()){
                        v.push_back(sub);
                        }                        
                    }
                    if(v[0].length()>0){
                        v[0] = v[0].substr(1,v[0].length()-1);
                    }
                    if(v[v.size()-1].length()>0){
                        v[v.size()-1] = v[v.size()-1].substr(0,v[v.size()-1].length()-1);
                    }
                    //v is a vector where each element is an element of each row starting at usb# and ending at the last gain.
                    output.push_back(v);       
                    //cout<<"Pushed line: "<<lineNum<<'\n';                    
                } 
        lineNum++;
        }
        //Print out vector for debug:
        /*for (unsigned int i = 0; i < output.size(); i++){
          cout << "Line: "<<i+1 << "\n";
          for (unsigned int j = 0; j < output[i].size(); j++){
          cout<<"Value "<<j+1<<" :"<<output[i][j]<<"\n";
          }
        }*/
        *numberofboardused = output.size();
	//cout << "Number of boards used: " << *numberofboardused << endl;
        for(int i = 0; i<output.size(); i++){
            pmtboardnumber[i+1] = atoi(output[i][2].c_str()); //PMT board number is the 3rd parameter in the fcl file
            //cout << "pmtboardnumber[" << i+1 << "]: " << pmtboardnumber[i+1] << endl;
            for(int j=11; j<output[i].size(); j++){   //Gains start at the 12th parameter in the fcl file
                gainused[pmtboardnumber[i+1]][j-11] = atof(output[i][j].c_str());
                //cout << "gain for board " << pmtboardnumber[i+1] << ", channel " << j-11 << " is " << output[i][j] << endl;
            }
        }
	return;
        file.close();
        keepReading = false;
    }
    else{
        
        cout << "Unable to open file. Using hardcoded values.";
	*numberofboardused = 4;
	pmtboardnumber[1]=3;
	pmtboardnumber[2]=7;
	pmtboardnumber[3]=1;
	pmtboardnumber[4]=2;
        return;
        
    }
}
