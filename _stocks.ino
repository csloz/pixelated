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

void DoStocks() {

     //DoStocks Code Here...

  GetStock("TSLA", "84CN3AUMFGSI2U6K");
  delay (15000);//15 seconds
}


int GetStock (const char *stock_name, const char *stock_apikey) {
  String strCall;

//  strCall = "https://www.alphavantage.co/query?function=GLOBAL_QUOTE&symbol=TSLA&interval=5min&apikey=84CN3AUMFGSI2U6K"
  strCall = "https://www.alphavantage.co/query?function=GLOBAL_QUOTE&symbol=" + String(stock_name) + "&interval=5min&apikey=" + String(stock_apikey);
 

  //Lets setup a http client
  HTTPClient http;
    
  http.begin (strCall);
  int httpCode = http.GET();

  if (httpCode) {  //Success?

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
       DEBUG_PRINTLN ("**HERE**");
       
       //DEBUG_PRINTLN (doc["01. symbol"]);
       //DEBUG_PRINTLN (doc["05. price"]);
       matrix.print ("STOCK:");
       const char* str =doc[0];
       matrix.print (str);
       matrix.print (" $ ");
       const char* str2 = doc[4];
       matrix.print (str2); 
       
       DEBUG_PRINTLN ("**PAST**");
       
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
