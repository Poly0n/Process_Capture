#include "catch_amalgamated.hpp"
#include "Execution.h"

TEST_CASE("isNumber accepts valid numeric strings") {
	Execution exec;
	REQUIRE(exec.isNumber("1"));
	REQUIRE(exec.isNumber("10"));
	REQUIRE(exec.isNumber("65535"));

}

TEST_CASE("isNumber rejects empty or non-numeric input") {
	Execution exec;
	REQUIRE_FALSE(exec.isNumber(""));
	REQUIRE_FALSE(exec.isNumber("abc"));
	REQUIRE_FALSE(exec.isNumber("12a3"));
	REQUIRE_FALSE(exec.isNumber(" "));
}

TEST_CASE("isNumber rejects leading zeros") {
	Execution exec;

	REQUIRE_FALSE(exec.isNumber("01"));
	REQUIRE_FALSE(exec.isNumber("001"));
}

TEST_CASE("isNumber enforces numeric bounds") {
	Execution exec;

	REQUIRE_FALSE(exec.isNumber("65536"));   // > max allowed
	REQUIRE_FALSE(exec.isNumber("99999999")); // too large
}

TEST_CASE("isNumber rejects overly long input") {
	Execution exec;

	REQUIRE_FALSE(exec.isNumber("12345678901")); // > 10 chars
}

TEST_CASE("wordCommands identifies multi-word commands") {
	Execution exec;

	REQUIRE(exec.wordCommands("find"));
	REQUIRE(exec.wordCommands("filter"));
}

TEST_CASE("wordCommands rejects invalid commands") {
	Execution exec;

	REQUIRE_FALSE(exec.wordCommands("help"));
	REQUIRE_FALSE(exec.wordCommands("kill"));
}

TEST_CASE("oneLinerCommand identifies valid single commands") {
	Execution exec;

	REQUIRE(exec.oneLinerCommand("help"));
	REQUIRE(exec.oneLinerCommand("refresh"));
	REQUIRE(exec.oneLinerCommand("q"));
	REQUIRE(exec.oneLinerCommand("Q"));
}

TEST_CASE("oneLinerCommand rejects invalid input") {
	Execution exec;

	REQUIRE_FALSE(exec.oneLinerCommand("find"));
	REQUIRE_FALSE(exec.oneLinerCommand(""));
}
