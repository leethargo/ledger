#include "reconcile.h"

namespace ledger {

reconcile_results_t reconcile_account(journal_t&     journal,
				      account_t&     account,
				      const value_t& balance)
{
  // This routine attempts to reconcile an account against a given
  // `balance' by marking transactions as "cleared" until the cleared
  // balance matches the expected `balance'.
  //
  // The real difficulty is that sometimes there are transactions in
  // the journal which never make it to the statement (they might be
  // drawing from the wrong account), or there could be transactions
  // which haven't been added yet.  In both of these cases the best
  // one can do is guess, and if that fails, to throw up their hands
  // in despair.
  //
  // As such, this algorithm is very likely to fail.  The hope is that
  // sometimes it won't fail, and then it can save the user a fair bit
  // of time.
  //
  // If the algorithm succeeds in auto-reconciling the account, then
  // all the relevant data is return in the form of a
  // `reconcile_results_t' structure (see reconcile.h).

  // Compute the current balances for the given account.
  value_t cleared_balance;
  value_t pending_balance;

  reconcile_results_t results;
  transactions_list   pending_xacts;

  for (entries_list::iterator e = journal.entries.begin();
       e != journal.entries.end();
       e++)
    for (transactions_list::iterator x = (*e)->transactions.begin();
	 x != (*e)->transactions.end();
	 x++)
      if ((*x)->account == &account) {
	switch ((*e)->state) {
	case entry_t::CLEARED:
	  cleared_balance += (*x)->amount;
	  break;
	case entry_t::UNCLEARED:
	case entry_t::PENDING:
	  pending_balance += (*x)->amount;
	  pending_xacts.push_back(*x);
	  break;
	}
      }

  results.previous_balance = cleared_balance;

  value_t to_reconcile = balance - cleared_balance;

  // If the amount to reconcile is the same as the pending balance,
  // then assume an exact match and return the results right away.
  if (to_reconcile == pending_balance) {
    results.remaining_balance = 0L;
    results.pending_xacts     = pending_xacts;
    return results;
  }

  throw error("Could not reconcile account");
}

} // namespace ledger
