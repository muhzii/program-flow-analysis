//
// Created by muhzi on 6/1/20.
//

#include "value.h"

ValueRecord UNIT_VALUE_RECORD = {
        .type  = SYMVAL_TYPE_INTEGER,
        .value = { ._i = 1, },
};

static int is_op_computable (ValueRecord vr1, ValueRecord vr2);
static ValueType get_common_type (ValueRecord vr1, ValueRecord vr2);
static Value wrap_value (double value, ValueType type);

double
value_record_get_value (ValueRecord vr) {
    switch (vr.type) {
        case SYMVAL_TYPE_INTEGER:
            return vr.value._i;
        case SYMVAL_TYPE_FLOAT:
            return vr.value._f;
        case SYMVAL_TYPE_CHAR:
            return vr.value._c;
        case SYMVAL_TYPE_UNKNOWN:
            return 0;
    }
}

ValueRecord
value_record_add (ValueRecord vr1, ValueRecord vr2) {
    if (!is_op_computable(vr1, vr2))
        return NO_VALUE_RECORD;

    ValueType type = get_common_type(vr1, vr2);
    double res = value_record_get_value(vr1) + value_record_get_value(vr2);
    return (ValueRecord) {
            .type  = type,
            .value = wrap_value(res, type),
    };
}

ValueRecord
value_record_sub (ValueRecord vr1, ValueRecord vr2) {
    if (!is_op_computable(vr1, vr2))
        return NO_VALUE_RECORD;

    ValueType type = get_common_type(vr1, vr2);
    double res = value_record_get_value(vr1) - value_record_get_value(vr2);
    return (ValueRecord) {
            .type  = type,
            .value = wrap_value(res, type),
    };
}

ValueRecord
value_record_mul (ValueRecord vr1, ValueRecord vr2) {
    if (!is_op_computable(vr1, vr2))
        return NO_VALUE_RECORD;

    ValueType type = get_common_type(vr1, vr2);
    double res = value_record_get_value(vr1) * value_record_get_value(vr2);
    return (ValueRecord) {
            .type  = type,
            .value = wrap_value(res, type),
    };
}

ValueRecord
value_record_dev (ValueRecord vr1, ValueRecord vr2) {
    if (!is_op_computable(vr1, vr2))
        return NO_VALUE_RECORD;

    ValueType type = get_common_type(vr1, vr2);
    double res = value_record_get_value(vr1) / value_record_get_value(vr2);
    return (ValueRecord) {
            .type  = type,
            .value = wrap_value(res, type),
    };
}

ValueRecord
value_record_rem (ValueRecord vr1, ValueRecord vr2) {
    if (vr1.type != SYMVAL_TYPE_INTEGER || vr2.type != SYMVAL_TYPE_INTEGER)
        return NO_VALUE_RECORD;

    ValueType type = get_common_type(vr1, vr2);
    double res = vr1.value._i % vr2.value._i;
    return (ValueRecord) {
            .type  = type,
            .value = wrap_value(res, type),
    };
}

static int
is_op_computable (ValueRecord vr1, ValueRecord vr2) {
    return vr1.type != SYMVAL_TYPE_UNKNOWN &&
                vr2.type != SYMVAL_TYPE_UNKNOWN;
}

static ValueType
get_common_type (ValueRecord vr1, ValueRecord vr2) {
    if (vr1.type == SYMVAL_TYPE_UNKNOWN || vr2.type == SYMVAL_TYPE_UNKNOWN) {
        return SYMVAL_TYPE_UNKNOWN;
    }

    if (vr1.type == SYMVAL_TYPE_FLOAT || vr2.type == SYMVAL_TYPE_FLOAT) {
        return SYMVAL_TYPE_FLOAT;
    } else if (vr1.type == SYMVAL_TYPE_INTEGER || vr2.type == SYMVAL_TYPE_INTEGER) {
        return SYMVAL_TYPE_INTEGER;
    } else {
        return SYMVAL_TYPE_CHAR;
    }
}

static Value
wrap_value (double value, ValueType type)
{
    switch (type) {
        case SYMVAL_TYPE_FLOAT:
            return (Value) { ._f = value, };
        case SYMVAL_TYPE_INTEGER:
            return (Value) { ._i = (int)value, };
        case SYMVAL_TYPE_CHAR:
            return (Value) { ._c = (char)value, };
        case SYMVAL_TYPE_UNKNOWN:
            return (Value) { };
    }
}
