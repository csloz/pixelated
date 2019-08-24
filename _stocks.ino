/*
//www.quandl.com/api/v3/datasets/WIKI/TSLA.json?rows=1
//Pull 1 day of details for TSLA

{"dataset":{
"id":11136412,
"dataset_code":"TSLA",
"database_code":"WIKI",
"name":"Tesla Motors Inc. (TSLA) Prices, Dividends, Splits and Trading Volume",
"description":"\u003cp\u003eEnd of day open, high, low, close and volume, dividends and splits, and split/dividend adjusted open, high, low close and volume for Tesla Motors, Inc. (TSLA). Ex-Dividend is non-zero on ex-dividend dates. Split Ratio is 1 on non-split dates. Adjusted prices are calculated per CRSP (\u003ca href=\"http://www.crsp.com/products/documentation/crsp-calculations\" rel=\"nofollow\" target=\"blank\"\u003ewww.crsp.com/products/documentation/crsp-calculations\u003c/a\u003e)\u003c/p\u003e\n\n\u003cp\u003eThis data is in the public domain. You may copy, distribute, disseminate or include the data in other products for commercial and/or noncommercial purposes.\u003c/p\u003e\n\n\u003cp\u003eThis data is part of Quandl's Wiki initiative to get financial data permanently into the public domain. Quandl relies on users like you to flag errors and provide data where data is wrong or missing. Get involved: \u003ca href=\"mailto:connect@quandl.com\" rel=\"nofollow\" target=\"blank\"\u003econnect@quandl.com\u003c/a\u003e\n\u003c/p\u003e\n",
"refreshed_at":"2018-03-27T21:46:05.593Z",
"newest_available_date":"2018-03-27",
"oldest_available_date":"2010-06-29",
"column_names":["Date","Open","High","Low","Close","Volume","Ex-Dividend","Split Ratio","Adj. Open","Adj. High","Adj. Low","Adj. Close","Adj. Volume"],"frequency":"daily","type":"Time Series",
"premium":false,"limit":1,"transform":null,"column_index":null,
"start_date":"2010-06-29",
"end_date":"2018-03-27",
"data":[["2018-03-27",304.0,304.27,277.18,279.18,13696168.0,0.0,1.0,304.0,304.27,277.18,279.18,13696168.0]],
"collapse":null,
"order":null,
"database_id":4922}}

//Use arduinojson.org/v6/assistant to generate code.  Its awesome!


https://www.alphavantage.co/query?function=GLOBAL_QUOTE&symbol=TSLA&interval=5min&apikey=84CN3AUMFGSI2U6K



*/

int moon_ico [50] = {
  //3 nuances: 0x18c3 < 0x3186 < 0x4a49
  0x0000, 0x4a49, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x18c3,
  0x0000, 0x0000, 0x0000, 0x4a49, 0x3186, 0x3186, 0x18c3, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0000, 0x4a49, 0x4a49, 0x3186, 0x3186, 0x3186, 0x18c3, 0x0000, 0x0000,
  0x0000, 0x0000, 0x4a49, 0x3186, 0x3186, 0x3186, 0x3186, 0x18c3, 0x0000, 0x0000,
};


int sunny_ico [50] = {
  0x0000, 0x0000, 0x0000, 0xffe0, 0x0000, 0x0000, 0xffe0, 0x0000, 0x0000, 0x0000,
  0x0000, 0xffe0, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xffe0, 0x0000,
  0x0000, 0x0000, 0x0000, 0xffe0, 0xffe0, 0xffe0, 0xffe0, 0x0000, 0x0000, 0x0000,
  0xffe0, 0x0000, 0xffe0, 0xffe0, 0xffe0, 0xffe0, 0xffe0, 0xffe0, 0x0000, 0xffe0,
  0x0000, 0x0000, 0xffe0, 0xffe0, 0xffe0, 0xffe0, 0xffe0, 0xffe0, 0x0000, 0x0000,
};


struct stock {
  char stock_symbol[10]="TSLA";  //Default to TESLA as I own stock ;)
  float stock_open =  232.9900;
  float stock_high = 235.7700;
  float stock_low = 228.7500;
  float stock_price =  235.00;
  long stock_volume =  4663937;
  const char* stock_latest_trading_day = "2019-08-12";
  float stock_previous_close =  235.0100;
  float stock_change =  5.99;
  float stock_change_percent = 2.62;
};

stock Stocks;

int currentCursor = COLUMNS;
int offset =COLUMNS;

String strText = "Calling NewsAPI.org... ";   

void displayStockScroller(const String &strText){
     String  t="";
     int bits = strText.length();
     const int width=18;
     
      //Show scroller at bottom 48
      matrix.setFont(&Dialog_plain);
      matrix.setTextSize (1);
      matrix.setTextColor(WHITE);
      matrix.setTextWrap (false);
      //matrix.print ("Text Scroller Location");

        offset++;

      //Smooth scrolling, do pixel move for each char, then shift..
      
        if (offset < strText.length()) {
            //Have text to pump into the blitter?
            for (int i=0; i < width; i++)
                t+= strText.charAt((offset +i) % strText.length());   
        }
        else {
             //Reset scroller
             offset =0;
             for (int i=0; i < width; i++)
                t+= strText.charAt((offset +i) % strText.length()); 
        }
        matrix.setCursor (0,55);
        matrix.print ( t );
        


     /*   for (int x=64, i=0; ; x--l) {
          
          for (int x2=x, i2=i; x2<64) {
           int8_t w = Font::Draw (test[i2], x2, 0);
           x2 += w, i2 = (i2+1) % strlen(test);
           if (x2 <= 0)  // off the display completely?
             x = x2, i = i2;
           }
           delay(500);
        }
        //matrix.setScrollMode (wrapForward);*/
        
}

unsigned long time_now =0;
unsigned long time_now2 =0;
int Delay = 1 * 1000; //Polling time for Stock (Stock API)
int HrDelay = 1 *1000; //1Hr Delay (News API)

void DoStocks() {
  
  //Poll Stocks every Delay seconds.
  if (millis() > time_now + Delay) {  //First time in millis will enter loop, after that will delay 
          time_now=millis();
          DEBUG_PRINTLN();
          DEBUG_PRINT ("DoStocksPre[");
          DEBUG_PRINT (Stocks.stock_symbol );
          DEBUG_PRINTLN ("]");
          if (getStock(Stocks.stock_symbol, "84CN3AUMFGSI2U6K")) { 
            DEBUG_PRINT ("DO STOCKS POST [");
            DEBUG_PRINT (Stocks.stock_symbol);
            DEBUG_PRINTLN ("]");
            Delay = 300 * 1000; //Change to 5 min after success.
          }
  }
  
  /*if (millis() > time_now + HrDelay) {  //First time in millis will enter loop, after that will delay 
      if (getNews ("https://newsapi.org/v2/top-headlines?country=us&category=business&pageSize=20&apiKey=","dabccea193474ecbb178825d19fa52bf")) {
          DEBUG_PRINTLN ("Got News");
          HrDelay = 60*60*1000; //Set delay to 1hr
      }
  }
  */
  if (millis() > time_now2) { //Only scroll every 300ms
    time_now2 =millis()+300;
    matrix.fillRect (0,45,64,20, BLACK);
    displayStockScroller(strText);
  }
  
}


void setStock (const char *stock_symbol) {

      DEBUG_PRINT ("SET STOCK [");
       DEBUG_PRINT (Stocks.stock_symbol);
       DEBUG_PRINTLN ("]");
       DEBUG_PRINT (".");
       size_t Size = strlen(stock_symbol);
       DEBUG_PRINT (Size);
       DEBUG_PRINT (".");
       //Stocks.stock_symbol = malloc(Size);
       DEBUG_PRINT (".");
       strcpy (Stocks.stock_symbol , stock_symbol);
       DEBUG_PRINT (".");
       
       Delay = 1*1000; //reset delay
       
}


int getStock (const char * stock_symbol,const char *stock_apikey) {
  String strCall;
  int err=0;

  DEBUG_PRINTLN ();
  DEBUG_PRINT ("Get Stock [");
  DEBUG_PRINT (Stocks.stock_symbol);
  DEBUG_PRINTLN ("]");

  
  //return(true);
  
//  strCall = "https://www.alphavantage.co/query?function=GLOBAL_QUOTE&symbol=TSLA&interval=5min&apikey=8 4CN 3AU MFG SI2 U6K"
  strCall = "https://www.alphavantage.co/query?function=GLOBAL_QUOTE&symbol=" + String(stock_symbol) + "&interval=5min&apikey=" + String(stock_apikey);
 
  DEBUG_PRINTLN (strCall);

 // return(true);
 
  #define DISABLE_STOCK_FALSE //as API dead and need to test..
  #ifdef DISABLE_STOCK_FALSE
  
    //Lets setup a http client 
    HTTPClient http;
    
    http.begin (strCall);
    int httpCode = http.GET();

  #else 
    int  httpCode= true;
  #endif
  
 
  if (httpCode>0) {  //Success?
       #ifdef DISABLE_STOCK_FALSE
       DEBUG_PRINTLN();
       DEBUG_PRINT ("HTTP CODE");   DEBUG_PRINT (httpCode);  DEBUG_PRINT (" STATUS "); DEBUG_PRINTLN(err);
       DEBUG_PRINTLN (strCall);
       delay (1000); //delay 1 seconds
       
       String payload = http.getString();
       
       DEBUG_PRINT ("Payload ["); DEBUG_PRINT (payload); DEBUG_PRINTLN ("]");

       //Got Payload?  Lets parse
       const size_t capacity = JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(10) + 400;
       DynamicJsonDocument doc(capacity);
       
       DeserializationError error= deserializeJson (doc, payload);
       
       if (error) { //Failed JSON
          DEBUG_PRINTLN ("[HTTP] Json Parse Failed");
          DEBUG_PRINTLN (error.c_str());
          return (false);
       }

       JsonObject stockQuote = doc ["Global Quote"];

       #endif
       
       if (stockQuote.isNull() ) { //Failure?

         DEBUG_PRINTLN ("[HTTP] Json Object Failure");
         DEBUG_PRINTLN (error.c_str());
         return( false);
         
       }
       else {   //Success Update the values from JSON
         Stocks.stock_price = stockQuote ["05. price"].as<float>();
         
         strcpy (Stocks.stock_symbol , stockQuote ["01. symbol"]);
  
         Stocks.stock_change = stockQuote ["09. change"].as<float>();
         Stocks.stock_change_percent = stockQuote ["10. change percent"].as<float>();
       }
       
       //Draw Stock Symbol
       matrix.setFont(&Picopixel);
       matrix.clear();
       matrix.setTextColor  (WHITE);
       matrix.setCursor (2,15);
       matrix.setTextSize (2);
       
       matrix.println (Stocks.stock_symbol);

       //Draw Stock Value
       if (Stocks.stock_change>0) {
          matrix.setTextColor  (GREEN);
       } else {
          matrix.setTextColor (RED);
       }
       //matrix.drawFastHLine(2, 18, 6, 0xFEFE);
       matrix.setCursor (1,30);
       //matrix.print ("$ ");
       
       matrix.println ( Stocks.stock_price,2); 
             

       //Draw Change
       matrix.setTextColor  (WHITE);
       matrix.setTextSize (1);
       matrix.setCursor (1,40);
       matrix.print ( Stocks.stock_change,2);

       //Draw Triangle (up / down)
       if (Stocks.stock_change>0) {
            matrix.fillTriangle (48,31,52, 25, 56, 31, GREEN); 
       }
       else {
          matrix.fillTriangle (52, 31, 48,25, 56,25, RED); 
       }
        
       //Draw Change %
       matrix.setCursor (32,40);
       matrix.print ( Stocks.stock_change_percent,2);
       matrix.print ("%");

       //Draw Moon or Day Icon
       //if market open show sun, else show moon.  need to add in ntpdate to get time 
       matrix.setCursor (48,2);
       matrix.drawIcon ( sunny_ico, 48, 2, 10, 5);
       return (true);
  }
  else { //Failed HTTP
    DEBUG_PRINTLN ("[HTTP] Failed to get Stock");
    return (false);
  }

   //Here, so HTTP get and Parse must have worked!
   
  /*const char* stock_symbol = jsonBuffer["01. symbol"]; // "TSLA"
  const char* stock_open = root["02. open"]; // "232.9900"
  const char* stock_high = root["03. high"]; // "235.7700"
  const char* stock_low = root["04. low"]; // "228.7500"
  const char* stock_price = root["05. price"]; // "229.0100"
  const char* stock_volume = root["06. volume"]; // "4663937"
  const char* stock_latest_trading_day = root["07. latest trading day"]; // "2019-08-12"
  const char* stock_previous_close = root["08. previous close"]; // "235.0100"
  const char* stock_change = root["09. change"]; // "-6.0000"
  const char* stock_change_percent = root["10. change percent"]; // "-2.5531%"
  */

 // matrix.print (doc["01. symbol"].as<char*>());
 // matrix.print (doc["05. price"].as<char*>()); 
  
}



int getNews (const char *apiname, const char *apikey) {
    String strCall;
    int err=0;
    //  strCall = "https://newsapi.org/v2/top-headlines?country=us&category=business&pageSize=10&apiKey=dabccea193474ecbb178825d19fa52bf" //limit to 10 
    strCall = String (apiname) + String (apikey);
    
    //Lets setup a http client 
    HTTPClient http;
    
    http.begin (strCall);
    int httpCode = http.GET();

 
    if (httpCode>0) {  //Success?
       
       DEBUG_PRINTLN();
       DEBUG_PRINT ("HTTP CODE");   DEBUG_PRINT (httpCode);  DEBUG_PRINT (" STATUS "); DEBUG_PRINTLN(err);
       DEBUG_PRINTLN (strCall);
       delay (1000); //delay 1 seconds
       
       String payload = http.getString();
       
       //DEBUG_PRINT ("Payload ["); DEBUG_PRINT (payload); DEBUG_PRINTLN ("]");

       //Got Payload?  Lets parse
       const size_t capacity = JSON_ARRAY_SIZE(20) + 20*JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + 20*JSON_OBJECT_SIZE(8) + 19420; //Need to up later?
       DynamicJsonDocument doc(capacity);
       
       DeserializationError error= deserializeJson (doc, payload);
       
       if (error) { //Failed JSON
          DEBUG_PRINTLN ("[HTTP] Json Parse Failed");
          DEBUG_PRINTLN (error.c_str());
          return (false);
       }

       int totalResults = doc["totalResults"];
       
       JsonArray results = doc ["articles"];
       DEBUG_PRINTLN (totalResults);
       if (totalResults>20) {
          totalResults = 20;
        };  //limit to 10

       strText = " "; //Clear strText
       for (int i=0; i< totalResults; i++) {
           JsonObject article = results[i];
           //DEBUG_PRINTLN ("[NEWS] "); 
           //DEBUG_PRINT (i);
           strText+= article ["description"].as<String>(); //For now don't concat 
           strText+= " :: ";
           //DEBUG_PRINT (" ");
           //DEBUG_PRINTLN (strText);
       }
       DEBUG_PRINTLN ("[HTTP] Got News");
       return (true);
  }
  else { //Failed HTTP
    DEBUG_PRINTLN ("[HTTP] Failed to get News");
    return (false);
  }

  
}
