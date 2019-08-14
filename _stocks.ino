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

struct stock {
  const char* stock_symbol =  "TSLA";
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
int offset =0;

void displayStockScroller(){
     String strText = "Crop Production is up 6.5% today said China Daily.   Rioting in Hong Kong is due to those pesky bad people.   Good News everybody, its a beautiful day out there in China, nothing bad happened. ";
     String  t="";
     int bits = strText.length();
     const int width=18;
     
      //Show scroller at bottom 48
      
      matrix.setTextSize (1);
      matrix.setTextColor(RED);
      matrix.setTextWrap (false);
      //matrix.print ("Text Scroller Location");
/*
        offset++;

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
        matrix.setCursor (0,50);
        matrix.print ( t );
        //DEBUG_PRINTLN (t);*/

        matrix.setScrollMode (wrapForward);
        
}

unsigned long time_now =0;
unsigned long time_now2 =0;

void DoStocks() {
  int Delay = 30* 1000; //Polling time for Stock
  
  
  //Poll Stocks every Delay seconds.
  if (millis() > time_now + Delay) {  //First time in millis will enter loop, after that will delay 
          time_now=millis();
          getStock("TSLA", "84CN3AUMFGSI2U6K");
          //DEBUG_PRINTLN ("GOT STOCK");
  }
  if (millis() > time_now2) { //Only scroll every 200ms
    time_now2 =millis()+200;
    matrix.fillRect (0,45,64,20, BLACK);
    displayStockScroller();
  }
}



int getStock (const char *stock_name, const char *stock_apikey) {
  String strCall;

//  strCall = "https://www.alphavantage.co/query?function=GLOBAL_QUOTE&symbol=TSLA&interval=5min&apikey=8 4CN 3AU MFG SI2 U6K"
  strCall = "https://www.alphavantage.co/query?function=GLOBAL_QUOTE&symbol=" + String(stock_name) + "&interval=5min&apikey=" + String(stock_apikey);
 

  
  
//  #define DISABLE_STOCK_FALSE
  #ifdef DISABLE_STOCK_FALSE
  
    //Lets setup a http client 
    HTTPClient http;
    
  http.begin (strCall);
  int httpCode = http.GET();

  #else 
    int  httpCode= true;
  #endif
  
 
  if (httpCode) {  //Success?
       #ifdef DISABLE_STOCK_FALSE
        
       DEBUG_PRINTLN (strCall);
       String payload = http.getString();
       delay (200);
       DEBUG_PRINTLN (payload);

       //Got Payload?  Lets parse
       const size_t capacity = JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(10) + 260;
       DynamicJsonDocument doc(capacity);
       
       DeserializationError error= deserializeJson (doc, payload);
       
       if (error) { //Failed JSON
          DEBUG_PRINTLN ("[HTTP] Json Parse Failed");
          DEBUG_PRINTLN (error.c_str());
          return (false);
       }

       //JsonObject stockQuote = doc ["Global Quote"];

       #endif
       
      // DEBUG_PRINTLN (String( stockQuote ["01. symbol"]));
      // DEBUG_PRINTLN (String( stockQuote ["05. price"]));
      // DEBUG_PRINTLN stockQuote ["09. change"]
      // Save all the values to the stock struct.
      //float flt = stockQuote ["05. price"].as<float>();
       
       //Draw Stock Symbol
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

/*
 #define BLACK    0x0000
#define BLUE     0x001F
#define RED      0xF800
#define GREEN    0x07E0
#define CYAN     0x07FF
#define MAGENTA  0xF81F
#define YELLOW   0xFFE0 
#define WHITE    0xFFFF

#define LED_RED_VERYLOW   (3 <<  11)
#define LED_RED_LOW     (7 <<  11)
#define LED_RED_MEDIUM    (15 << 11)
#define LED_RED_HIGH    (31 << 11)

#define LED_GREEN_VERYLOW (1 <<  5)   
#define LED_GREEN_LOW     (15 << 5)  
#define LED_GREEN_MEDIUM  (31 << 5)  
#define LED_GREEN_HIGH    (63 << 5)  

#define LED_BLUE_VERYLOW  3
#define LED_BLUE_LOW    7
#define LED_BLUE_MEDIUM   15
#define LED_BLUE_HIGH     31


 */
