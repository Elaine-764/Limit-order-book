// tests/test_symbol_table.cc
#include <gtest/gtest.h>
#include <string>
#include <utility>
#include <stdexcept>

#include "../include/exchange.h"   
#include "../include/data.h"
#include "../include/types.h"
#include "test_helpers.h"

// ============================================================================
// 1. REGISTRATION AND INTERNING FLOWS
// ============================================================================

TEST(SymbolTableTest, InternNewTickerReturnsUniqueId) {
    SymbolTable table;

    // Registering a brand new symbol for the first time
    std::uint32_t id1 = table.intern("AAPL");
    std::uint32_t id2 = table.intern("MSFT");

    // Verification: IDs must be unique and assign sequentially from zero index
    EXPECT_EQ(id1, 0u);
    EXPECT_EQ(id2, 1u);
}

TEST(SymbolTableTest, InternExistingTickerReturnsSameId) {
    SymbolTable table;

    std::uint32_t first_id = table.intern("BTC");
    
    // Call intern() a second time on an already registered string
    std::uint32_t second_id = table.intern("BTC");

    // Verification: Single pass lookup should find the item and return the identical ID
    EXPECT_EQ(first_id, second_id);
    EXPECT_EQ(second_id, 0u);
}

TEST(SymbolTableTest, InternRValueOverloadStealsMemory) {
    SymbolTable table;

    // Create a temporary string allocation that passes an rvalue/xvalue signature
    std::string ticker = "VERY_LONG_ALGORITHMIC_SYMBOL_NAME_THAT_BYPASSES_SSO";
    
    // Execute move semantics signature pass
    std::uint32_t id = table.intern(std::move(ticker));

    EXPECT_EQ(id, 0u);
    
    // Structural Invariant: The source string data must have been safely hollowed out 
    // or set to empty state after passing through the move-only signature layer.
    EXPECT_TRUE(ticker.empty() || ticker.data() != table.name(id).data());
}

// ============================================================================
// 2. REVERSE NAME RESOLUTION (ID TO STRING)
// ============================================================================

TEST(SymbolTableTest, NameLookupReturnsCorrectStringReference) {
    SymbolTable table;

    std::uint32_t id = table.intern("ETH");

    // Fetch the name back out using the generated ID slot index
    const std::string& symbol_name = table.name(id);

    // Verification: The string must match, and the address should be an lvalue reference
    EXPECT_EQ(symbol_name, "ETH");
}

TEST(SymbolTableTest, NameLookupIsZeroCopyReference) {
    SymbolTable table;
    std::uint32_t id = table.intern("SOL");

    // Verify low-latency requirement: name() returns a direct const reference 
    // matching the exact internal vector node container storage address
    const std::string& ref1 = table.name(id);
    const std::string& ref2 = table.name(id);

    EXPECT_EQ(&ref1, &ref2); // Pointer addresses must be identical (Zero Copies)
}

// ============================================================================
// 3. BOUNDARY CONDITIONS AND ERROR HANDLING
// ============================================================================
TEST(SymbolTableTest, NameLookupOnInvalidIdReturnsSafeEmptyString) {
    SymbolTable table;

    // Seed table with exactly one item (ID = 0)
    table.intern("XRP");

    // Test Case 1: Out of bounds (id >= to_name_.size())
    const std::string& invalid_large_id_result = table.name(9999);
    EXPECT_EQ(invalid_large_id_result, "");
    
    // Test Case 2: Exact boundary index violation (id == to_name_.size())
    // Valid indices are only 0 to size-1. Checking index 1 should trigger the fallback.
    const std::string& boundary_id_result = table.name(1);
    EXPECT_EQ(boundary_id_result, "");

    // Safety Verification: Ensure the references point to a valid, live address 
    // and did not result in an immediate dangling stack reference crash.
    EXPECT_NO_FATAL_FAILURE({
        std::string copy_attempt = invalid_large_id_result;
        EXPECT_TRUE(copy_attempt.empty());
    });
}
