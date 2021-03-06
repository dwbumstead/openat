/* Copyright 2017 Paolo Galeone <nessuno@nerdz.eu>. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.*/

#ifndef AT_KRAKEN_H_
#define AT_KRAKEN_H_

#include <at/crypt/namespace.hpp>
#include <at/exceptions.hpp>
#include <at/market.hpp>
#include <at/types.hpp>
#include <cerrno>
#include <chrono>
#include <ctime>
#include <limits>
#include <sstream>

namespace at {

/* Client for Kraken API.
 * API doumentation available: https://www.kraken.com/help/api
 * Method descriptions are kept from that page.
 *
 * Every method can throw a response_error or a server_error.
 * A response_error is when the API handles the request but for some reason
 * an error occuurs.
 *
 * A server_error is when the status code of the request is != 200.
 *
 * Margin trading is too risky and thus is not supported. */

class Kraken : public Market, private Thrower {
    // class Kraken : private Thrower {
private:
    const std::string _version = "0";
    const std::string _host = "https://api.kraken.com/" + _version + "/";
    const std::string _api_key, _api_secret;
    std::string _otp;
    std::vector<std::string> _available_symbols;

    const std::map<std::string, double> _minimumLimits = {
        // https://support.kraken.com/hc/en-us/articles/205893708-What-is-the-minimum-order-size-
        {"REP", 0.3},   {"XBT", 0.002}, {"BTC", 0.002}, {"BCH", 0.002},
        {"DASH", 0.03}, {"DOGE", 3000}, {"EOS", 3},     {"ETH", 0.02},
        {"ETC", 0.3},   {"GNO", 0.03},  {"ICN", 2},     {"LTC", 0.1},
        {"MLN", 0.1},   {"XMR", 0.1},   {"XRP", 30},    {"XLM", 300},
        {"ZEC", 0.03},  {"USDT", 5}};

    // Returns available symbols
    std::vector<std::string> _symbols();

    // Kraken uses XBT while other uses BTC.
    // Replace inputs symbol BTC with XBT
    void _sanitize_pair(currency_pair_t& pair);

    // Returns the minimum amount tradable for the specified currency
    double _minTradable(const std::string& symbol);

    // Converts a XXXYYY string in a currenty_pair_t, if XXX and YYY are known
    // symbols
    currency_pair_t _str2pair(std::string str);

    // _nonce = [0prefix || timestamp]<width = 10> || [nanoseconds]<width = 9>
    std::string _nonce() const;

    // base64encode(
    //  hmac_sha512(path + sha256(nonce + postdata),
    //   base64decode(_api_key))
    // )
    std::string _sign(const std::string& path, const std::string& nonce,
                      const std::string& postdata) const;

    // Authenticated post request
    json _request(std::string method,
                  std::vector<std::pair<std::string, std::string>> params);

public:
    Kraken() {}
    Kraken(std::string api_key, std::string api_secret)
        : _api_key(api_key), _api_secret(api_secret)
    {
    }
    Kraken(std::string api_key, std::string api_secret, std::string otp)
        : _api_key(api_key), _api_secret(api_secret), _otp(otp)
    {
    }
    ~Kraken() {}

    /* Set/Update OTP for private requests when 2FA is enabled */
    void set_otp(std::string otp) { _otp = otp; }

    /* Get server time
     * URL: https://api.kraken.com/0/public/Time
     *
     * Result: Server's time */
    std::time_t time() const;

    /* Get asset info
     * URL: https://api.kraken.com/0/public/Assets
     * Allows anyone to get a list of all the currencies that Kraken
     * currently supports at any given time. The list will include the name,
     * symbol, availability status, and an icon link for each. */
    std::map<std::string, coin_t> coins() override;

    /* Gets the current deposit info set by Kraken for the
     * specified currency. */
    deposit_info_t depositInfo(std::string currency) override;

    /* This gets the market info (pair, rate, limit, minimum limit, miner fee).
     */
    std::vector<market_info_t> info() override;

    /* This gets the market info (pair, limit, minimum limit, miner fee)
     * for the spcified currency. */
    market_info_t info(currency_pair_t) override;

    /* This gets the account balance, amount for every currency */
    std::map<std::string, double> balance() override;

    /* This gets the account balance for the specified currency */
    double balance(std::string currency) override;

    /* This gets the ticker for the specified pair at the current time */
    ticker_t ticker(currency_pair_t) override;

    /* This get the order book for the specicified pair */
    std::vector<ticker_t> orderBook(currency_pair_t) override;

    /* This get the complete closed orders */
    std::vector<order_t> closedOrders() override;

    /* This get the complete open orders */
    std::vector<order_t> openOrders() override;

    /* This adds an order using only the meaningful fields of order and filling
     * the remeaning fields once the order has been placed */
    void place(order_t&) override;

    /* This cancel the specified order idientified by order.txid */
    void cancel(order_t&) override;
};

}  // end namespace at

#endif  // AT_KRAKEN_H_
