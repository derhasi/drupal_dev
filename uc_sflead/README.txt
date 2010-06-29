; $Id$

Field Mapping
=============

Seperators:
- "|": divides definition of nested variables (array or object) in its levels
    e.g. $order->my_array['tree']['myfield'] => my_array|tree|myfield
- ",": seperates multiple fields to merge to one value
    e.g. "field1,field2,fieldX"
          will merge to array(valueOfField1, valueOfField2, valueOfFieldX)