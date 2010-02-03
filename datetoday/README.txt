//$Id$

This module provides an additional formatter for date fields. This formatter
is capable of providing Date strings like "yesterday - 18:00" or
"tomorrow - Friday, 9:00".

Configuration:
--------------
after activating the module, you can configure it on:
* admin/settings/date-time/formats/datetoday

There you can define what level shall has its own presentation, like 'last day'
for 'yesterday'. All dates that do not fit selected presentation will use the
'fallback' settings of the field.

Settings in 'default' will be used for new date fields.



The module integrates spans by default:
* this minute
* next minute
* last minute
* this hour
* next hour
* last hour
* this day => today
* next day => tomorrow
* last day => yesterday
* this week
* next week
* last week
* this month
* last month
* next month
* this Year
* last year
* next year