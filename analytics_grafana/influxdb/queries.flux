// 1. CO2 Trend with movingAverage filter
from(bucket: "iot_data_n")
  |> range(start: -30m)
  |> filter(fn: (r) => r._measurement == "environment")
  |> filter(fn: (r) => r._field == "co2")
  |> aggregateWindow(every: 1m, fn: mean, createEmpty: false)
  |> movingAverage(n: 5)

// 2. Multi-field correlation: CO2 vs Motion
from(bucket: "iot_data_n")
  |> range(start: -10m)
  |> filter(fn: (r) => r._measurement == "environment")
  |> filter(fn: (r) => r._field == "co2" or r._field == "motion_per_min")
  |> aggregateWindow(every: 20s, fn: mean, createEmpty: false)

// 3. Raw Data Pivot Table
from(bucket: "iot_data_n")
  |> range(start: -1h) 
  |> filter(fn: (r) => r._measurement == "environment")
  |> pivot(rowKey:["_time"], columnKey: ["_field"], valueColumn: "_value")
  |> drop(columns: ["_start", "_stop", "_measurement"])
  |> sort(columns: ["_time"], desc: true)