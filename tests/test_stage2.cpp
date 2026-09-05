#include "../src/orderbook.h"
#include <iostream>
#include <string>

static int testsRun = 0;
static int testsFailed = 0;
static std::string currentTest;

#define CHECK(cond) do { \
    testsRun++; \
    if (!(cond)) { \
        testsFailed++; \
        std::cout << "  FAIL [" << currentTest << "] " << #cond << "  (line " << __LINE__ << ")\n"; \
    } \
} while(0)

#define TEST(name) currentTest = name; std::cout << "-- " << name << " --\n"

// Small helpers so test bodies read cleanly
static double bid(const OrderBook& b) { double p = -1; b.bestBid(p); return p; }
static double ask(const OrderBook& b) { double p = -1; b.bestAsk(p); return p; }

int main()
{
    // ---------------------------------------------------------------
    TEST("1. Reject price below range (89.95, range is 90.00-110.00)");
    {
        OrderBook book;
        auto r = book.addOrder({0, Side::BUY, 89.95, 10, 0});
        CHECK(r.accepted == false);
        CHECK(r.trades.empty());
        CHECK(bid(book) == -1);  // nothing should have been rested
    }

    // ---------------------------------------------------------------
    TEST("2. Reject price above range (110.05)");
    {
        OrderBook book;
        auto r = book.addOrder({0, Side::SELL, 110.05, 10, 0});
        CHECK(r.accepted == false);
        CHECK(ask(book) == -1);
    }

    // ---------------------------------------------------------------
    TEST("3. Accept exact lower boundary (90.00) and upper boundary (110.00)");
    {
        OrderBook book;
        auto r1 = book.addOrder({0, Side::BUY, 90.00, 10, 0});
        auto r2 = book.addOrder({0, Side::SELL, 110.00, 10, 0});
        CHECK(r1.accepted == true);
        CHECK(r2.accepted == true);
        CHECK(bid(book) == 90.00);
        CHECK(ask(book) == 110.00);
    }

    // ---------------------------------------------------------------
    TEST("4. No match: single resting order produces no trades");
    {
        OrderBook book;
        auto r = book.addOrder({0, Side::BUY, 99.00, 50, 0});
        CHECK(r.accepted == true);
        CHECK(r.trades.empty());
        CHECK(bid(book) == 99.00);
        CHECK(ask(book) == -1);
    }

    // ---------------------------------------------------------------
    TEST("5. Exact full match: incoming qty == resting qty");
    {
        OrderBook book;
        book.addOrder({0, Side::SELL, 100.00, 30, 0});  
        book.printOrderBook();        // id 0
        auto r = book.addOrder({0, Side::BUY, 100.00, 30, 0}); 
        book.printOrderBook();
        CHECK(r.trades.size() == 1);
        CHECK(r.trades[0].buyOrderId == 1);
        CHECK(r.trades[0].sellOrderId == 0);
        CHECK(r.trades[0].quantity == 30);
        CHECK(r.trades[0].price == 100.00);
        CHECK(ask(book) == -1);   // fully consumed, level gone
        CHECK(bid(book) == -1);   // incoming also fully consumed, nothing rests
    }

    // ---------------------------------------------------------------
    TEST("6. Partial fill: incoming smaller than resting -> resting stays, reduced");
    {
        OrderBook book;
        book.addOrder({0, Side::SELL, 100.00, 50, 0});          // id 0, qty 50
        auto r = book.addOrder({0, Side::BUY, 100.00, 20, 0});  // id 1, qty 20
        CHECK(r.trades.size() == 1);
        CHECK(r.trades[0].quantity == 20);
        CHECK(ask(book) == 100.00);   // level still occupied
        CHECK(bid(book) == -1);       // incoming fully used up, nothing rests
    }

    // ---------------------------------------------------------------
    TEST("7. Resting fully consumed on the OTHER side: incoming bigger than one resting order, rests remainder");
    {
        OrderBook book;
        book.addOrder({0, Side::SELL, 100.00, 20, 0});          // id 0, qty 20
        auto r = book.addOrder({0, Side::BUY, 100.00, 50, 0});  // id 1, qty 50
        CHECK(r.trades.size() == 1);
        CHECK(r.trades[0].quantity == 20);
        CHECK(ask(book) == -1);       // ask side fully drained
        CHECK(bid(book) == 100.00);   // remaining 30 rests as a new bid
    }

    // ---------------------------------------------------------------
    TEST("8. FIFO within same price level: three sells at same tick, matched in arrival order");
    {
        OrderBook book;
        book.addOrder({0, Side::SELL, 100.00, 10, 0});  // id 0
        book.addOrder({0, Side::SELL, 100.00, 20, 0});  // id 1
        book.addOrder({0, Side::SELL, 100.00, 30, 0});  // id 2
        auto r = book.addOrder({0, Side::BUY, 100.00, 15, 0});  // id 3, only enough for id0 + partial id1
        CHECK(r.trades.size() == 2);
        CHECK(r.trades[0].sellOrderId == 0);
        CHECK(r.trades[0].quantity == 10);
        CHECK(r.trades[1].sellOrderId == 1);
        CHECK(r.trades[1].quantity == 5);   // partial: 15 - 10 = 5 taken from id1
    }

    // ---------------------------------------------------------------
    TEST("9. Cross multiple price levels: order eats through 2 ticks");
    {
        OrderBook book;
        book.addOrder({0, Side::SELL, 100.00, 10, 0});  // id 0
        book.addOrder({0, Side::SELL, 100.05, 10, 0});  // id 1
        auto r = book.addOrder({0, Side::BUY, 100.05, 15, 0});  // id 2
        CHECK(r.trades.size() == 2);
        CHECK(r.trades[0].price == 100.00);
        CHECK(r.trades[0].quantity == 10);
        CHECK(r.trades[1].price == 100.05);
        CHECK(r.trades[1].quantity == 5);
        CHECK(ask(book) == 100.05);   // 5 qty left resting on ask side at 100.05
    }

    // ---------------------------------------------------------------
    TEST("10. Partial-fill-then-rest across levels: leftover rests at incoming's own tick");
    {
        OrderBook book;
        book.addOrder({0, Side::SELL, 100.00, 10, 0});
        book.addOrder({0, Side::SELL, 100.05, 10, 0});
        auto r = book.addOrder({0, Side::BUY, 100.05, 100, 0});  // way more than available
        CHECK(r.trades.size() == 2);
        CHECK(ask(book) == -1);       // both ask levels fully drained
        CHECK(bid(book) == 100.05);   // remaining 80 rests at the incoming order's own price
    }

    // ---------------------------------------------------------------
    TEST("11. Price-time priority: trade price is the RESTING order's price, not incoming's");
    {
        OrderBook book;
        book.addOrder({0, Side::SELL, 99.50, 10, 0});           // resting sell at 99.50
        auto r = book.addOrder({0, Side::BUY, 100.00, 10, 0});  // aggressive buy at 100.00
        CHECK(r.trades.size() == 1);
        CHECK(r.trades[0].price == 99.50);   // must be the resting price, not 100.00
    }

    // ---------------------------------------------------------------
    TEST("12. No match when book empty on the other side (just rests)");
    {
        OrderBook book;
        auto r = book.addOrder({0, Side::BUY, 95.00, 10, 0});
        CHECK(r.trades.empty());
        CHECK(bid(book) == 95.00);
    }

    // ---------------------------------------------------------------
    TEST("13. No match when prices don't cross (ask above buy's limit)");
    {
        OrderBook book;
        book.addOrder({0, Side::SELL, 100.10, 10, 0});
        auto r = book.addOrder({0, Side::BUY, 100.00, 10, 0});  // 100.00 < 100.10, must not cross
        CHECK(r.trades.empty());
        CHECK(ask(book) == 100.10);
        CHECK(bid(book) == 100.00);
    }

    // ---------------------------------------------------------------
    TEST("14. Cancel HEAD of a multi-order level, list stays intact");
    {
        OrderBook book;
        book.addOrder({0, Side::BUY, 99.00, 10, 0});  // id 0 - head
        book.addOrder({0, Side::BUY, 99.00, 20, 0});  // id 1
        book.addOrder({0, Side::BUY, 99.00, 30, 0});  // id 2 - tail
        CHECK(book.cancelOrder(0) == true);
        // id0 gone, id1 should now be matchable first (FIFO preserved)
        auto r = book.addOrder({0, Side::SELL, 99.00, 20, 0});
        CHECK(r.trades.size() == 1);
        CHECK(r.trades[0].buyOrderId == 1);   // id1, not id0 (already cancelled) or id2 (out of order)
    }

    // ---------------------------------------------------------------
    TEST("15. Cancel TAIL of a multi-order level");
    {
        OrderBook book;
        book.addOrder({0, Side::BUY, 99.00, 10, 0});  // id 0
        book.addOrder({0, Side::BUY, 99.00, 20, 0});  // id 1
        book.addOrder({0, Side::BUY, 99.00, 30, 0});  // id 2 - tail
        CHECK(book.cancelOrder(2) == true);
        auto r = book.addOrder({0, Side::SELL, 99.00, 30, 0});
        CHECK(r.trades.size() == 2);          // must fill id0 then id1, id2 is gone
        CHECK(r.trades[0].buyOrderId == 0);
        CHECK(r.trades[1].buyOrderId == 1);
    }

    // ---------------------------------------------------------------
    TEST("16. Cancel MIDDLE of a multi-order level, neighbors relink correctly");
    {
        OrderBook book;
        book.addOrder({0, Side::BUY, 99.00, 10, 0});  // id 0
        book.addOrder({0, Side::BUY, 99.00, 20, 0});  // id 1 - middle
        book.addOrder({0, Side::BUY, 99.00, 30, 0});  // id 2
        CHECK(book.cancelOrder(1) == true);
        auto r = book.addOrder({0, Side::SELL, 99.00, 40, 0});
        CHECK(r.trades.size() == 2);          // id0 then id2, id1 skipped entirely
        CHECK(r.trades[0].buyOrderId == 0);
        CHECK(r.trades[0].quantity == 10);
        CHECK(r.trades[1].buyOrderId == 2);
        CHECK(r.trades[1].quantity == 30);
    }

    // ---------------------------------------------------------------
    TEST("17. Cancel the only order in a level empties it and updates best price");
    {
        OrderBook book;
        book.addOrder({0, Side::BUY, 99.00, 10, 0});   // id 0
        book.addOrder({0, Side::BUY, 100.00, 10, 0});  // id 1 - current best bid
        CHECK(bid(book) == 100.00);
        CHECK(book.cancelOrder(1) == true);
        CHECK(bid(book) == 99.00);   // best bid must advance down
    }

    // ---------------------------------------------------------------
    TEST("18. Best-tick advances correctly across a GAP (non-contiguous occupied ticks)");
    {
        OrderBook book;
        book.addOrder({0, Side::BUY, 100.00, 10, 0});  // id 0 - best
        book.addOrder({0, Side::BUY, 95.00, 10, 0});   // id 1 - gap of many empty ticks below
        CHECK(bid(book) == 100.00);
        CHECK(book.cancelOrder(0) == true);
        CHECK(bid(book) == 95.00);   // must correctly skip the empty gap and land on 95.00
    }

    // ---------------------------------------------------------------
    TEST("19. Cancel non-existent id returns false, no side effects");
    {
        OrderBook book;
        book.addOrder({0, Side::BUY, 99.00, 10, 0});
        CHECK(book.cancelOrder(999) == false);
        CHECK(bid(book) == 99.00);   // book untouched
    }

    // ---------------------------------------------------------------
    TEST("20. Cancel an id that was already fully matched away returns false");
    {
        OrderBook book;
        book.addOrder({0, Side::SELL, 100.00, 10, 0});          // id 0
        book.addOrder({0, Side::BUY, 100.00, 10, 0});           // id 1, fully consumes id 0
        CHECK(book.cancelOrder(0) == false);   // id 0 no longer in the index
    }

    // ---------------------------------------------------------------
    TEST("21. Draining an entire side leaves bestBid/bestAsk correctly empty");
    {
        OrderBook book;
        book.addOrder({0, Side::SELL, 100.00, 10, 0});  // id 0
        book.addOrder({0, Side::SELL, 100.05, 10, 0});  // id 1
        auto r = book.addOrder({0, Side::BUY, 100.05, 20, 0});  // exactly drains both
        CHECK(r.trades.size() == 2);
        CHECK(ask(book) == -1);
        CHECK(bid(book) == -1);   // exact fill, nothing rests either
    }

    // ---------------------------------------------------------------
    TEST("22. Repeated cancel of the same id fails the second time");
    {
        OrderBook book;
        book.addOrder({0, Side::BUY, 99.00, 10, 0});  // id 0
        CHECK(book.cancelOrder(0) == true);
        CHECK(book.cancelOrder(0) == false);   // already gone
    }

    // ---------------------------------------------------------------
    std::cout << "\n" << testsRun << " checks run, " << testsFailed << " failed.\n";
    return testsFailed == 0 ? 0 : 1;
}