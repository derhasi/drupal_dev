; $Id$

This module sends data from ubercart orders to salesforce. On any update the
defined keys are filled with values via field mapping (see below).
Settings have to be entered at admin/store/settings/orders/edit/sflead.

Target URL for submissions:
- https://www.salesforce.com/servlet/servlet.WebToLead?encoding=UTF-8

OpenSSL has to be enabled for that:
@see http://drupal.org/node/305498


Field Mapping
=============

Fixed values
------------
Simply return a predefined value.

Variable values
----------------
Seperators for variable value:
- "|": divides definition of nested variables (array or object) in its levels
    e.g. $order->my_array['tree']['myfield'] => my_array|tree|myfield
- ",": seperates multiple fields to merge to one value
    e.g. "field1,field2,fieldX"
          will merge to array(valueOfField1, valueOfField2, valueOfFieldX)

Token support
-------------
Global, order and user values kan be return via token replacements.

Debug Mode
===========
Salesforce provides a debug mode, that will send success messages to a defined
mail address. Therefore add two new fixed mappings.

sfkey       | mapping_type  | value
------------------------------
debug       | fixed         | 1
debugEmail  | fixed         | yourmail@example.com


