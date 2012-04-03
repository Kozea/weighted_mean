#include "postgres.h"
#include "fmgr.h"
#include "utils/memutils.h"
#include "utils/numeric.h"
#include "utils/builtins.h"


#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

PG_FUNCTION_INFO_V1(_weighted_mean_intermediate);
PG_FUNCTION_INFO_V1(_weighted_mean_final);


typedef struct WeightedMeanInternalState
{
	Numeric		running_sum;
	Numeric		running_amount;
	MemoryContext mcontext;
}	WeightedMeanInternalState;


Datum
_weighted_mean_final(PG_FUNCTION_ARGS)
{
	WeightedMeanInternalState *state;
	MemoryContext oldcontext;
	Datum		total;

	state = (WeightedMeanInternalState *) PG_GETARG_POINTER(0);
	oldcontext = MemoryContextSwitchTo(state->mcontext);
	total = DirectFunctionCall2(numeric_div, NumericGetDatum(state->running_sum), NumericGetDatum(state->running_amount));
	pfree(state);
	MemoryContextSwitchTo(oldcontext);
	PG_RETURN_NUMERIC(total);
}

Datum
_weighted_mean_intermediate(PG_FUNCTION_ARGS)
{
	WeightedMeanInternalState *state;
	Datum		value,
				amount,
				temp_total;
	MemoryContext aggcontext,
				oldcontext,
				internalcontext;


	if (!AggCheckCallContext(fcinfo, &aggcontext))
	{
		/* cannot be called directly because of internal-type argument */
		elog(ERROR, "_weighted_mean_intermediate called in non-aggregate context");
	}
	if (PG_ARGISNULL(0))
	{
		internalcontext = AllocSetContextCreate(aggcontext, "WeightedMeanState",
												ALLOCSET_DEFAULT_MINSIZE,
												ALLOCSET_DEFAULT_INITSIZE,
												ALLOCSET_DEFAULT_MAXSIZE);
		oldcontext = MemoryContextSwitchTo(internalcontext);
		state = (WeightedMeanInternalState *) palloc(sizeof(WeightedMeanInternalState));
		state->mcontext = internalcontext;
		state->running_sum = DatumGetNumeric(DirectFunctionCall1(numeric_in, CStringGetDatum("0")));
		state->running_amount = DatumGetNumeric(DirectFunctionCall1(numeric_in, CStringGetDatum("0")));
	}
	else
	{
		state = (WeightedMeanInternalState *) PG_GETARG_POINTER(0);
		oldcontext = MemoryContextSwitchTo(state->mcontext);
	}
	value = PG_GETARG_DATUM(1);
	amount = PG_GETARG_DATUM(2);
	temp_total = DirectFunctionCall2(numeric_mul, value, amount);
	state->running_sum = DatumGetNumeric(DirectFunctionCall2(numeric_add, NumericGetDatum(state->running_sum), temp_total));
	state->running_amount = DatumGetNumeric(DirectFunctionCall2(numeric_add,
							NumericGetDatum(state->running_amount), amount));
	MemoryContextSwitchTo(oldcontext);
	PG_RETURN_POINTER(state);
}
