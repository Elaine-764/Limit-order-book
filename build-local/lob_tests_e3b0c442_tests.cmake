add_test([=[PriceLevelTest.DefaultConstructorStartsEmptyWithZeroQty]=]  /Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local/lob_tests [==[--gtest_filter=PriceLevelTest.DefaultConstructorStartsEmptyWithZeroQty]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[PriceLevelTest.DefaultConstructorStartsEmptyWithZeroQty]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/tests/test_price_level.cc:7]==]
    WORKING_DIRECTORY [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[PriceLevelTest.PriceConstructorSetsPriceAndStartsEmpty]=]  /Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local/lob_tests [==[--gtest_filter=PriceLevelTest.PriceConstructorSetsPriceAndStartsEmpty]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[PriceLevelTest.PriceConstructorSetsPriceAndStartsEmpty]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/tests/test_price_level.cc:13]==]
    WORKING_DIRECTORY [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[PriceLevelTest.AddOrderIncreasesQtyAndClearsEmpty]=]  /Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local/lob_tests [==[--gtest_filter=PriceLevelTest.AddOrderIncreasesQtyAndClearsEmpty]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[PriceLevelTest.AddOrderIncreasesQtyAndClearsEmpty]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/tests/test_price_level.cc:20]==]
    WORKING_DIRECTORY [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[PriceLevelTest.MultipleAddOrdersAccumulateQty]=]  /Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local/lob_tests [==[--gtest_filter=PriceLevelTest.MultipleAddOrdersAccumulateQty]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[PriceLevelTest.MultipleAddOrdersAccumulateQty]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/tests/test_price_level.cc:28]==]
    WORKING_DIRECTORY [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[PriceLevelTest.AddOrderReturnsIteratorToJustInsertedOrder]=]  /Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local/lob_tests [==[--gtest_filter=PriceLevelTest.AddOrderReturnsIteratorToJustInsertedOrder]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[PriceLevelTest.AddOrderReturnsIteratorToJustInsertedOrder]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/tests/test_price_level.cc:37]==]
    WORKING_DIRECTORY [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[PriceLevelTest.RemoveOrderDecreasesQtyByThatOrdersAmount]=]  /Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local/lob_tests [==[--gtest_filter=PriceLevelTest.RemoveOrderDecreasesQtyByThatOrdersAmount]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[PriceLevelTest.RemoveOrderDecreasesQtyByThatOrdersAmount]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/tests/test_price_level.cc:46]==]
    WORKING_DIRECTORY [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[PriceLevelTest.RemoveMiddleOrderLeavesOthersIntact]=]  /Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local/lob_tests [==[--gtest_filter=PriceLevelTest.RemoveMiddleOrderLeavesOthersIntact]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[PriceLevelTest.RemoveMiddleOrderLeavesOthersIntact]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/tests/test_price_level.cc:57]==]
    WORKING_DIRECTORY [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[PriceLevelTest.RemovingLastOrderMakesLevelEmpty]=]  /Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local/lob_tests [==[--gtest_filter=PriceLevelTest.RemovingLastOrderMakesLevelEmpty]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[PriceLevelTest.RemovingLastOrderMakesLevelEmpty]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/tests/test_price_level.cc:72]==]
    WORKING_DIRECTORY [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[PriceLevelTest.RemovingAllOrdersOneByOneLeavesConsistentQty]=]  /Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local/lob_tests [==[--gtest_filter=PriceLevelTest.RemovingAllOrdersOneByOneLeavesConsistentQty]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[PriceLevelTest.RemovingAllOrdersOneByOneLeavesConsistentQty]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/tests/test_price_level.cc:82]==]
    WORKING_DIRECTORY [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[PriceLevelTest.OrdersPreserveFifoInsertionOrder]=]  /Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local/lob_tests [==[--gtest_filter=PriceLevelTest.OrdersPreserveFifoInsertionOrder]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[PriceLevelTest.OrdersPreserveFifoInsertionOrder]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/tests/test_price_level.cc:95]==]
    WORKING_DIRECTORY [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[PriceLevelTest.FifoOrderSurvivesAMiddleRemoval]=]  /Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local/lob_tests [==[--gtest_filter=PriceLevelTest.FifoOrderSurvivesAMiddleRemoval]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[PriceLevelTest.FifoOrderSurvivesAMiddleRemoval]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/tests/test_price_level.cc:104]==]
    WORKING_DIRECTORY [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[OrderBookTest.RestingBuyOrderBecomesBestBid]=]  /Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local/lob_tests [==[--gtest_filter=OrderBookTest.RestingBuyOrderBecomesBestBid]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[OrderBookTest.RestingBuyOrderBecomesBestBid]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/tests/test_order_book.cc:12]==]
    WORKING_DIRECTORY [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[OrderBookTest.RestingSellOrderBecomesBestAsk]=]  /Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local/lob_tests [==[--gtest_filter=OrderBookTest.RestingSellOrderBecomesBestAsk]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[OrderBookTest.RestingSellOrderBecomesBestAsk]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/tests/test_order_book.cc:18]==]
    WORKING_DIRECTORY [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[OrderBookTest.BidsOrderedHighestFirst]=]  /Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local/lob_tests [==[--gtest_filter=OrderBookTest.BidsOrderedHighestFirst]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[OrderBookTest.BidsOrderedHighestFirst]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/tests/test_order_book.cc:24]==]
    WORKING_DIRECTORY [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[OrderBookTest.AsksOrderedLowestFirst]=]  /Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local/lob_tests [==[--gtest_filter=OrderBookTest.AsksOrderedLowestFirst]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[OrderBookTest.AsksOrderedLowestFirst]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/tests/test_order_book.cc:32]==]
    WORKING_DIRECTORY [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[OrderBookTest.MultipleOrdersAtSamePriceAccumulateQty]=]  /Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local/lob_tests [==[--gtest_filter=OrderBookTest.MultipleOrdersAtSamePriceAccumulateQty]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[OrderBookTest.MultipleOrdersAtSamePriceAccumulateQty]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/tests/test_order_book.cc:40]==]
    WORKING_DIRECTORY [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[OrderBookTest.CancelRemovesOrderFromIndex]=]  /Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local/lob_tests [==[--gtest_filter=OrderBookTest.CancelRemovesOrderFromIndex]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[OrderBookTest.CancelRemovesOrderFromIndex]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/tests/test_order_book.cc:48]==]
    WORKING_DIRECTORY [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[OrderBookTest.CancelingOnlyOrderAtPriceRemovesTheLevel]=]  /Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local/lob_tests [==[--gtest_filter=OrderBookTest.CancelingOnlyOrderAtPriceRemovesTheLevel]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[OrderBookTest.CancelingOnlyOrderAtPriceRemovesTheLevel]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/tests/test_order_book.cc:59]==]
    WORKING_DIRECTORY [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[OrderBookTest.CancelingOneOfSeveralOrdersLeavesLevelAndSiblingsIntact]=]  /Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local/lob_tests [==[--gtest_filter=OrderBookTest.CancelingOneOfSeveralOrdersLeavesLevelAndSiblingsIntact]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[OrderBookTest.CancelingOneOfSeveralOrdersLeavesLevelAndSiblingsIntact]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/tests/test_order_book.cc:68]==]
    WORKING_DIRECTORY [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[OrderBookTest.EmptyingBestPriceRevealsNextBestBid]=]  /Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local/lob_tests [==[--gtest_filter=OrderBookTest.EmptyingBestPriceRevealsNextBestBid]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[OrderBookTest.EmptyingBestPriceRevealsNextBestBid]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/tests/test_order_book.cc:81]==]
    WORKING_DIRECTORY [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[OrderBookTest.CancelingUnknownOrderIdReturnsFalseNotCrash]=]  /Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local/lob_tests [==[--gtest_filter=OrderBookTest.CancelingUnknownOrderIdReturnsFalseNotCrash]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[OrderBookTest.CancelingUnknownOrderIdReturnsFalseNotCrash]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/tests/test_order_book.cc:91]==]
    WORKING_DIRECTORY [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[OrderBookTest.CancelingSameOrderTwiceIsSafeAndReturnsFalseSecondTime]=]  /Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local/lob_tests [==[--gtest_filter=OrderBookTest.CancelingSameOrderTwiceIsSafeAndReturnsFalseSecondTime]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[OrderBookTest.CancelingSameOrderTwiceIsSafeAndReturnsFalseSecondTime]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/tests/test_order_book.cc:96]==]
    WORKING_DIRECTORY [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[OrderBookTest.BidsAndAsksAreIndependent]=]  /Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local/lob_tests [==[--gtest_filter=OrderBookTest.BidsAndAsksAreIndependent]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[OrderBookTest.BidsAndAsksAreIndependent]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/tests/test_order_book.cc:104]==]
    WORKING_DIRECTORY [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[StopOrderBookTest.EmptyBookTriggersNothing]=]  /Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local/lob_tests [==[--gtest_filter=StopOrderBookTest.EmptyBookTriggersNothing]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[StopOrderBookTest.EmptyBookTriggersNothing]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/tests/test_stop_order_book.cc:7]==]
    WORKING_DIRECTORY [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[StopOrderBookTest.BuyStopTriggersWhenPriceRisesToItsStopPrice]=]  /Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local/lob_tests [==[--gtest_filter=StopOrderBookTest.BuyStopTriggersWhenPriceRisesToItsStopPrice]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[StopOrderBookTest.BuyStopTriggersWhenPriceRisesToItsStopPrice]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/tests/test_stop_order_book.cc:13]==]
    WORKING_DIRECTORY [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[StopOrderBookTest.BuyStopDoesNotTriggerBelowItsStopPrice]=]  /Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local/lob_tests [==[--gtest_filter=StopOrderBookTest.BuyStopDoesNotTriggerBelowItsStopPrice]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[StopOrderBookTest.BuyStopDoesNotTriggerBelowItsStopPrice]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/tests/test_stop_order_book.cc:25]==]
    WORKING_DIRECTORY [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[StopOrderBookTest.OnlyBuyStopsAtOrBelowLastTradeTrigger]=]  /Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local/lob_tests [==[--gtest_filter=StopOrderBookTest.OnlyBuyStopsAtOrBelowLastTradeTrigger]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[StopOrderBookTest.OnlyBuyStopsAtOrBelowLastTradeTrigger]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/tests/test_stop_order_book.cc:34]==]
    WORKING_DIRECTORY [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[StopOrderBookTest.SellStopTriggersWhenPriceFallsToItsStopPrice]=]  /Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local/lob_tests [==[--gtest_filter=StopOrderBookTest.SellStopTriggersWhenPriceFallsToItsStopPrice]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[StopOrderBookTest.SellStopTriggersWhenPriceFallsToItsStopPrice]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/tests/test_stop_order_book.cc:45]==]
    WORKING_DIRECTORY [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[StopOrderBookTest.SellStopDoesNotTriggerAbovePriceItsStopPrice]=]  /Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local/lob_tests [==[--gtest_filter=StopOrderBookTest.SellStopDoesNotTriggerAbovePriceItsStopPrice]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[StopOrderBookTest.SellStopDoesNotTriggerAbovePriceItsStopPrice]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/tests/test_stop_order_book.cc:55]==]
    WORKING_DIRECTORY [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[StopOrderBookTest.OnlySellStopsAtOrAboveLastTradeTrigger]=]  /Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local/lob_tests [==[--gtest_filter=StopOrderBookTest.OnlySellStopsAtOrAboveLastTradeTrigger]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[StopOrderBookTest.OnlySellStopsAtOrAboveLastTradeTrigger]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/tests/test_stop_order_book.cc:64]==]
    WORKING_DIRECTORY [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[StopOrderBookTest.TriggeredOrdersAreConsumedNotReTriggered]=]  /Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local/lob_tests [==[--gtest_filter=StopOrderBookTest.TriggeredOrdersAreConsumedNotReTriggered]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[StopOrderBookTest.TriggeredOrdersAreConsumedNotReTriggered]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/tests/test_stop_order_book.cc:75]==]
    WORKING_DIRECTORY [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[StopOrderBookTest.BuyAndSellStopsAreIndependent]=]  /Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local/lob_tests [==[--gtest_filter=StopOrderBookTest.BuyAndSellStopsAreIndependent]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[StopOrderBookTest.BuyAndSellStopsAreIndependent]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/tests/test_stop_order_book.cc:86]==]
    WORKING_DIRECTORY [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[SymbolTableTest.InternNewTickerReturnsUniqueId]=]  /Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local/lob_tests [==[--gtest_filter=SymbolTableTest.InternNewTickerReturnsUniqueId]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[SymbolTableTest.InternNewTickerReturnsUniqueId]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/tests/test_symbol_table.cc:16]==]
    WORKING_DIRECTORY [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[SymbolTableTest.InternExistingTickerReturnsSameId]=]  /Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local/lob_tests [==[--gtest_filter=SymbolTableTest.InternExistingTickerReturnsSameId]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[SymbolTableTest.InternExistingTickerReturnsSameId]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/tests/test_symbol_table.cc:28]==]
    WORKING_DIRECTORY [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[SymbolTableTest.InternRValueOverloadStealsMemory]=]  /Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local/lob_tests [==[--gtest_filter=SymbolTableTest.InternRValueOverloadStealsMemory]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[SymbolTableTest.InternRValueOverloadStealsMemory]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/tests/test_symbol_table.cc:41]==]
    WORKING_DIRECTORY [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[SymbolTableTest.NameLookupReturnsCorrectStringReference]=]  /Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local/lob_tests [==[--gtest_filter=SymbolTableTest.NameLookupReturnsCorrectStringReference]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[SymbolTableTest.NameLookupReturnsCorrectStringReference]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/tests/test_symbol_table.cc:61]==]
    WORKING_DIRECTORY [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[SymbolTableTest.NameLookupIsZeroCopyReference]=]  /Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local/lob_tests [==[--gtest_filter=SymbolTableTest.NameLookupIsZeroCopyReference]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[SymbolTableTest.NameLookupIsZeroCopyReference]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/tests/test_symbol_table.cc:73]==]
    WORKING_DIRECTORY [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[SymbolTableTest.NameLookupOnInvalidIdReturnsSafeEmptyString]=]  /Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local/lob_tests [==[--gtest_filter=SymbolTableTest.NameLookupOnInvalidIdReturnsSafeEmptyString]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[SymbolTableTest.NameLookupOnInvalidIdReturnsSafeEmptyString]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/tests/test_symbol_table.cc:88]==]
    WORKING_DIRECTORY [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[MatchingEngineCurrentTest.LimitOrderMatchesAtPriceTimePriority]=]  /Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local/lob_tests [==[--gtest_filter=MatchingEngineCurrentTest.LimitOrderMatchesAtPriceTimePriority]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[MatchingEngineCurrentTest.LimitOrderMatchesAtPriceTimePriority]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/tests/test_matching_engine_v2.cc:7]==]
    WORKING_DIRECTORY [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[MatchingEngineCurrentTest.MarketOrderSweepsMultipleLevelsAndCancelsRemainder]=]  /Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local/lob_tests [==[--gtest_filter=MatchingEngineCurrentTest.MarketOrderSweepsMultipleLevelsAndCancelsRemainder]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[MatchingEngineCurrentTest.MarketOrderSweepsMultipleLevelsAndCancelsRemainder]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/tests/test_matching_engine_v2.cc:30]==]
    WORKING_DIRECTORY [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[MatchingEngineCurrentTest.IOCLimitCancelsOnlyUnfilledRemainder]=]  /Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local/lob_tests [==[--gtest_filter=MatchingEngineCurrentTest.IOCLimitCancelsOnlyUnfilledRemainder]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[MatchingEngineCurrentTest.IOCLimitCancelsOnlyUnfilledRemainder]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/tests/test_matching_engine_v2.cc:49]==]
    WORKING_DIRECTORY [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[MatchingEngineCurrentTest.FOKRejectsWithoutConsumingInsufficientLiquidity]=]  /Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local/lob_tests [==[--gtest_filter=MatchingEngineCurrentTest.FOKRejectsWithoutConsumingInsufficientLiquidity]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[MatchingEngineCurrentTest.FOKRejectsWithoutConsumingInsufficientLiquidity]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/tests/test_matching_engine_v2.cc:69]==]
    WORKING_DIRECTORY [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[MatchingEngineCurrentTest.AllOrNoneLimitRejectsWithoutConsumingLiquidity]=]  /Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local/lob_tests [==[--gtest_filter=MatchingEngineCurrentTest.AllOrNoneLimitRejectsWithoutConsumingLiquidity]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[MatchingEngineCurrentTest.AllOrNoneLimitRejectsWithoutConsumingLiquidity]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/tests/test_matching_engine_v2.cc:88]==]
    WORKING_DIRECTORY [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[MatchingEngineCurrentTest.UnfilledGTCOrderRestsInBook]=]  /Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local/lob_tests [==[--gtest_filter=MatchingEngineCurrentTest.UnfilledGTCOrderRestsInBook]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[MatchingEngineCurrentTest.UnfilledGTCOrderRestsInBook]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/tests/test_matching_engine_v2.cc:105]==]
    WORKING_DIRECTORY [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[MatchingEngineCurrentTest.BuyStopTriggersAndIsRemovedFromStopBook]=]  /Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local/lob_tests [==[--gtest_filter=MatchingEngineCurrentTest.BuyStopTriggersAndIsRemovedFromStopBook]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[MatchingEngineCurrentTest.BuyStopTriggersAndIsRemovedFromStopBook]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/tests/test_matching_engine_v2.cc:120]==]
    WORKING_DIRECTORY [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[MatchingEngineCurrentTest.SellStopsTriggerForPricesAtOrBelowTheirThreshold]=]  /Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local/lob_tests [==[--gtest_filter=MatchingEngineCurrentTest.SellStopsTriggerForPricesAtOrBelowTheirThreshold]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[MatchingEngineCurrentTest.SellStopsTriggerForPricesAtOrBelowTheirThreshold]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/tests/test_matching_engine_v2.cc:140]==]
    WORKING_DIRECTORY [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[ExchangeCurrentTest.LazilyCreatesBookAndCancelsRestingOrder]=]  /Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local/lob_tests [==[--gtest_filter=ExchangeCurrentTest.LazilyCreatesBookAndCancelsRestingOrder]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[ExchangeCurrentTest.LazilyCreatesBookAndCancelsRestingOrder]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/tests/test_exchange_v2.cc:6]==]
    WORKING_DIRECTORY [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[ExchangeCurrentTest.MarketOrderWithZeroPriceIsAccepted]=]  /Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local/lob_tests [==[--gtest_filter=ExchangeCurrentTest.MarketOrderWithZeroPriceIsAccepted]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[ExchangeCurrentTest.MarketOrderWithZeroPriceIsAccepted]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/tests/test_exchange_v2.cc:20]==]
    WORKING_DIRECTORY [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[ExchangeCurrentTest.InvalidQuantityAndLimitPriceAreRejected]=]  /Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local/lob_tests [==[--gtest_filter=ExchangeCurrentTest.InvalidQuantityAndLimitPriceAreRejected]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[ExchangeCurrentTest.InvalidQuantityAndLimitPriceAreRejected]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/tests/test_exchange_v2.cc:35]==]
    WORKING_DIRECTORY [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[ExchangeCurrentTest.QuantityReductionModificationPreservesOrder]=]  /Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local/lob_tests [==[--gtest_filter=ExchangeCurrentTest.QuantityReductionModificationPreservesOrder]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[ExchangeCurrentTest.QuantityReductionModificationPreservesOrder]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/tests/test_exchange_v2.cc:49]==]
    WORKING_DIRECTORY [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[ExchangeCurrentTest.EndOfDayCancelsDayOrdersButKeepsGTCOrders]=]  /Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local/lob_tests [==[--gtest_filter=ExchangeCurrentTest.EndOfDayCancelsDayOrdersButKeepsGTCOrders]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[ExchangeCurrentTest.EndOfDayCancelsDayOrdersButKeepsGTCOrders]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/tests/test_exchange_v2.cc:61]==]
    WORKING_DIRECTORY [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[ExchangeCurrentTest.EndOfDayCancelsDayStopOrders]=]  /Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local/lob_tests [==[--gtest_filter=ExchangeCurrentTest.EndOfDayCancelsDayStopOrders]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[ExchangeCurrentTest.EndOfDayCancelsDayStopOrders]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/tests/test_exchange_v2.cc:80]==]
    WORKING_DIRECTORY [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[ExchangeCurrentTest.UnknownCancelAndModifyAreRejected]=]  /Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local/lob_tests [==[--gtest_filter=ExchangeCurrentTest.UnknownCancelAndModifyAreRejected]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[ExchangeCurrentTest.UnknownCancelAndModifyAreRejected]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/tests/test_exchange_v2.cc:93]==]
    WORKING_DIRECTORY [==[/Users/lijing/Desktop/Personal/Coding/Quant/limit_order_book/build-local]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
set(lob_tests_TESTS [==[PriceLevelTest.DefaultConstructorStartsEmptyWithZeroQty]==] [==[PriceLevelTest.PriceConstructorSetsPriceAndStartsEmpty]==] [==[PriceLevelTest.AddOrderIncreasesQtyAndClearsEmpty]==] [==[PriceLevelTest.MultipleAddOrdersAccumulateQty]==] [==[PriceLevelTest.AddOrderReturnsIteratorToJustInsertedOrder]==] [==[PriceLevelTest.RemoveOrderDecreasesQtyByThatOrdersAmount]==] [==[PriceLevelTest.RemoveMiddleOrderLeavesOthersIntact]==] [==[PriceLevelTest.RemovingLastOrderMakesLevelEmpty]==] [==[PriceLevelTest.RemovingAllOrdersOneByOneLeavesConsistentQty]==] [==[PriceLevelTest.OrdersPreserveFifoInsertionOrder]==] [==[PriceLevelTest.FifoOrderSurvivesAMiddleRemoval]==] [==[OrderBookTest.RestingBuyOrderBecomesBestBid]==] [==[OrderBookTest.RestingSellOrderBecomesBestAsk]==] [==[OrderBookTest.BidsOrderedHighestFirst]==] [==[OrderBookTest.AsksOrderedLowestFirst]==] [==[OrderBookTest.MultipleOrdersAtSamePriceAccumulateQty]==] [==[OrderBookTest.CancelRemovesOrderFromIndex]==] [==[OrderBookTest.CancelingOnlyOrderAtPriceRemovesTheLevel]==] [==[OrderBookTest.CancelingOneOfSeveralOrdersLeavesLevelAndSiblingsIntact]==] [==[OrderBookTest.EmptyingBestPriceRevealsNextBestBid]==] [==[OrderBookTest.CancelingUnknownOrderIdReturnsFalseNotCrash]==] [==[OrderBookTest.CancelingSameOrderTwiceIsSafeAndReturnsFalseSecondTime]==] [==[OrderBookTest.BidsAndAsksAreIndependent]==] [==[StopOrderBookTest.EmptyBookTriggersNothing]==] [==[StopOrderBookTest.BuyStopTriggersWhenPriceRisesToItsStopPrice]==] [==[StopOrderBookTest.BuyStopDoesNotTriggerBelowItsStopPrice]==] [==[StopOrderBookTest.OnlyBuyStopsAtOrBelowLastTradeTrigger]==] [==[StopOrderBookTest.SellStopTriggersWhenPriceFallsToItsStopPrice]==] [==[StopOrderBookTest.SellStopDoesNotTriggerAbovePriceItsStopPrice]==] [==[StopOrderBookTest.OnlySellStopsAtOrAboveLastTradeTrigger]==] [==[StopOrderBookTest.TriggeredOrdersAreConsumedNotReTriggered]==] [==[StopOrderBookTest.BuyAndSellStopsAreIndependent]==] [==[SymbolTableTest.InternNewTickerReturnsUniqueId]==] [==[SymbolTableTest.InternExistingTickerReturnsSameId]==] [==[SymbolTableTest.InternRValueOverloadStealsMemory]==] [==[SymbolTableTest.NameLookupReturnsCorrectStringReference]==] [==[SymbolTableTest.NameLookupIsZeroCopyReference]==] [==[SymbolTableTest.NameLookupOnInvalidIdReturnsSafeEmptyString]==] [==[MatchingEngineCurrentTest.LimitOrderMatchesAtPriceTimePriority]==] [==[MatchingEngineCurrentTest.MarketOrderSweepsMultipleLevelsAndCancelsRemainder]==] [==[MatchingEngineCurrentTest.IOCLimitCancelsOnlyUnfilledRemainder]==] [==[MatchingEngineCurrentTest.FOKRejectsWithoutConsumingInsufficientLiquidity]==] [==[MatchingEngineCurrentTest.AllOrNoneLimitRejectsWithoutConsumingLiquidity]==] [==[MatchingEngineCurrentTest.UnfilledGTCOrderRestsInBook]==] [==[MatchingEngineCurrentTest.BuyStopTriggersAndIsRemovedFromStopBook]==] [==[MatchingEngineCurrentTest.SellStopsTriggerForPricesAtOrBelowTheirThreshold]==] [==[ExchangeCurrentTest.LazilyCreatesBookAndCancelsRestingOrder]==] [==[ExchangeCurrentTest.MarketOrderWithZeroPriceIsAccepted]==] [==[ExchangeCurrentTest.InvalidQuantityAndLimitPriceAreRejected]==] [==[ExchangeCurrentTest.QuantityReductionModificationPreservesOrder]==] [==[ExchangeCurrentTest.EndOfDayCancelsDayOrdersButKeepsGTCOrders]==] [==[ExchangeCurrentTest.EndOfDayCancelsDayStopOrders]==] [==[ExchangeCurrentTest.UnknownCancelAndModifyAreRejected]==])
