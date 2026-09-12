# Telemetry Processing & Analytics Layer

This directory contains the pipeline for real-time telemetry streaming, time-series storage, and analytical dashboards.

## Data Pipeline
1. **Ingestion (Node-RED)**: Subscribes to `home/elevator/status` over MQTT, parses incoming JSON payloads, and handles data validation.
2. **Preprocessing (`filter_moving_average.js`)**:
   - Normalizes status fields to numeric scales.
   - Applies a moving average filter (window size = 5) to mitigate MQ-135 sensor noise and voltage fluctuations.
   - Tracks session context across fragmented packets.
3. **Storage (InfluxDB v2)**: Writes normalized metrics into the `iot_data_n` bucket (`environment` measurement) with sub-second precision.
4. **Visualization (Grafana)**:
   - Analytical time-series dashboards leveraging custom Flux queries.
   - Moving average trend evaluation and correlation analysis (`CO2` vs `Motion`).
   - Gauge panels for operational load and raw telemetry audit tables.

## Directory Structure
- `nodered/`: Contains the exported flow (`flow.json`) and the standalone moving-average filter logic.
- `influxdb/`: Production Flux queries used for aggregation, multi-field filtering, and pivoting.
- `grafana/screenshots/`: Visual captures of the operational dashboard panels.