/*
 * Copyright (c) 2006-Present, Redis Ltd.
 * All rights reserved.
 *
 * Licensed under your choice of the Redis Source Available License 2.0
 * (RSALv2); or (b) the Server Side Public License v1 (SSPLv1); or (c) the
 * GNU Affero General Public License v3 (AGPLv3).
*/

#[macro_export]
macro_rules! _assert_match {
    ($pattern:expr, $expected_results:expr $(,)?, $outcome:expr) => {{
        let tokens = WildcardPattern::parse($pattern);

        let results: &[&[u8]] = &$expected_results;
        for expected in results {
            assert_eq!(
                tokens.matches(expected),
                $outcome,
                r#"Unexpected match outcome for {:?} when trying to match against {:?}"#,
                String::from_utf8_lossy(expected),
                String::from_utf8_lossy($pattern)
            );
        }
    }};
}

#[macro_export]
// For consistency, this macro should be called `match!`, but `match`
// is a keyword in Rust, so we use `matches!` instead.
macro_rules! matches {
    ($pattern:expr, $expected_results:expr $(,)?) => {{ $crate::_assert_match!($pattern, $expected_results, wildcard::MatchOutcome::Match) }};
}

#[macro_export]
macro_rules! no_match {
    ($pattern:expr, $expected_results:expr $(,)?) => {{ $crate::_assert_match!($pattern, $expected_results, wildcard::MatchOutcome::NoMatch) }};
}

#[macro_export]
macro_rules! partial_match {
    ($pattern:expr, $expected_results:expr $(,)?) => {{
        $crate::_assert_match!(
            $pattern,
            $expected_results,
            wildcard::MatchOutcome::PartialMatch
        )
    }};
}
