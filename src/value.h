//
// Created by muhzi on 6/1/20.
//

#ifndef FLOW_ANALYSIS_VALUE_H
#define FLOW_ANALYSIS_VALUE_H

enum ValueType {
    SYMVAL_TYPE_UNKNOWN,
    SYMVAL_TYPE_INTEGER,
    SYMVAL_TYPE_FLOAT,
    SYMVAL_TYPE_CHAR,
};

union Value {
    int _i;
    char _c;
    double _f;
};

// A record for a symbol's value - used to propagate constants when possible.
struct ValueRecord {
    enum ValueType type;
    union Value value;
};

typedef enum ValueType ValueType;
typedef union Value Value;
typedef struct ValueRecord ValueRecord;

ValueRecord NO_VALUE_RECORD;
ValueRecord UNIT_VALUE_RECORD;

double value_record_get_value (ValueRecord vr);
ValueRecord value_record_add (ValueRecord vr1, ValueRecord vr2);
ValueRecord value_record_sub (ValueRecord vr1, ValueRecord vr2);
ValueRecord value_record_mul (ValueRecord vr1, ValueRecord vr2);
ValueRecord value_record_dev (ValueRecord vr1, ValueRecord vr2);
ValueRecord value_record_rem (ValueRecord vr1, ValueRecord vr2);

#endif //FLOW_ANALYSIS_VALUE_H
