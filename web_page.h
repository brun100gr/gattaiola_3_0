#ifndef WEB_PAGE_H
#define WEB_PAGE_H

String html = R"HTML(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32 Configuration</title>
  <style>
    /* === RESET AND BASE STYLES === */
    /* Remove default margins and padding, set consistent box-sizing */
    * {
      margin: 0;
      padding: 0;
      box-sizing: border-box;
    }
    
    /* Main body style with gradient background */
    body {
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      min-height: 100vh;
      padding: 20px;
    }
    
    /* === MAIN LAYOUT === */
    /* Main centered container with shadow */
    .container {
      max-width: 800px;
      margin: 0 auto;
      background: white;
      border-radius: 15px;
      box-shadow: 0 10px 30px rgba(0,0,0,0.3);
      overflow: hidden;
    }
    
    /* Header with blue gradient */
    .header {
      background: linear-gradient(45deg, #2196F3, #21CBF3);
      color: white;
      padding: 30px;
      text-align: center;
    }
    
    /* Main title in header */
    .header h1 {
      font-size: 2.5em;
      margin-bottom: 10px;
    }
    
    /* Main content area */
    .content {
      padding: 30px;
    }
    
    /* === SECTIONS === */
    /* Each section (Status, Scheduled Action, WiFi Networks) */
    .section {
      margin-bottom: 30px;
      padding: 20px;
      background: #f8f9fa;
      border-radius: 10px;
      border-left: 4px solid #2196F3; /* Left colored border */
    }
    
    /* Section titles */
    .section h2 {
      color: #333;
      margin-bottom: 15px;
      font-size: 1.4em;
    }
    
    /* === INFORMATION GRID (System Status) === */
    /* Responsive grid for system information */
    .info-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
      gap: 15px;
      margin-bottom: 20px;
    }
    
    /* Single information item */
    .info-item {
      background: white;
      padding: 15px;
      border-radius: 8px;
      border: 1px solid #e0e0e0;
    }
    
    /* Information label (e.g. "WiFi Status") */
    .info-label {
      font-weight: bold;
      color: #666;
      font-size: 0.9em;
      margin-bottom: 5px;
    }
    
    /* Information value (e.g. "Connected") */
    .info-value {
      font-size: 1.2em;
      color: #333;
    }
    
    /* === FORM ELEMENTS === */
    /* Form element group */
    .form-group {
      margin-bottom: 20px;
    }
    
    /* Form labels */
    .form-group label {
      display: block;
      margin-bottom: 5px;
      font-weight: bold;
      color: #333;
    }
    
    /* Input fields */
    .form-group input {
      width: 100%;
      padding: 12px;
      border: 2px solid #e0e0e0;
      border-radius: 6px;
      font-size: 16px;
      transition: border-color 0.3s;
    }
    
    /* Focus state for inputs */
    .form-group input:focus {
      outline: none;
      border-color: #2196F3;
    }
    
    /* === BUTTONS === */
    /* Main button with blue gradient */
    .btn {
      background: linear-gradient(45deg, #2196F3, #21CBF3);
      color: white;
      border: none;
      padding: 12px 24px;
      border-radius: 6px;
      cursor: pointer;
      font-size: 16px;
      transition: transform 0.2s;
    }
    
    /* Hover effect on buttons */
    .btn:hover {
      transform: translateY(-2px);
    }
    
    /* === NETWORK ITEMS === */
    /* Container for each WiFi network */
    .network-item {
      background: white;
      border: 1px solid #e0e0e0;
      border-radius: 8px;
      padding: 15px;
      margin-bottom: 10px;
    }
    
    /* Network item header with status indicator */
    .network-header {
      display: flex;
      justify-content: space-between;
      align-items: center;
      margin-bottom: 10px;
    }
    
    /* Network status indicator (green/red dot) */
    .status-indicator {
      width: 12px;
      height: 12px;
      border-radius: 50%;
      background: #4CAF50; /* Green for enabled */
    }
    
    /* Red indicator for disabled networks */
    .status-indicator.disabled {
      background: #f44336;
    }
    
    /* Grid layout for network controls (SSID, Password, Enable, Delete) */
    .network-controls {
      display: grid;
      grid-template-columns: 1fr 1fr auto auto;
      gap: 10px;
      align-items: center;
    }
    
    /* Delete button with red gradient */
    .btn-delete {
      background: linear-gradient(45deg, #f44336, #ff6b6b);
      color: white;
      border: none;
      padding: 6px 12px;
      border-radius: 4px;
      cursor: pointer;
      font-size: 12px;
      transition: transform 0.2s;
    }
    
    /* Hover effect for delete button */
    .btn-delete:hover {
      transform: translateY(-1px);
    }
    
    /* Small button variant */
    .btn-small {
      padding: 6px 12px;
      font-size: 14px;
    }
    
    /* Large time display for current system time */
    .time-display {
      text-align: center;
      font-size: 2.5em;
      color: #2196F3;
      font-weight: bold;
      font-family: monospace;
      margin: 20px 0;
    }
    
    /* Display for scheduled action time */
    .scheduled-time {
      text-align: center;
      font-size: 1.8em;
      color: #4CAF50;
      margin: 15px 0;
    }
  </style>
</head>
<body>
  <!-- Main container for the entire interface -->
  <div class="container">
    <!-- Header section with title and description -->
    <div class="header">
      <h1>ESP32 Configuration Panel</h1>
      <p>Configure your device settings</p>
    </div>
    
    <!-- Main content area -->
    <div class="content">
      <!-- System Status Section -->
      <div class="section">
        <h2>System Status</h2>
        <!-- Grid layout for system information -->
        <div class="info-grid">
          <!-- WiFi connection status -->
          <div class="info-item">
            <div class="info-label">WiFi Status</div>
            <div class="info-value" id="wifi-status">Loading...</div>
          </div>
          <!-- Current IP address -->
          <div class="info-item">
            <div class="info-label">IP Address</div>
            <div class="info-value" id="ip-address">Loading...</div>
          </div>
          <!-- System uptime -->
          <div class="info-item">
            <div class="info-label">Uptime</div>
            <div class="info-value" id="uptime">Loading...</div>
          </div>
          <!-- Available memory -->
          <div class="info-item">
            <div class="info-label">Free Memory</div>
            <div class="info-value" id="free-memory">Loading...</div>
          </div>
        </div>
        <!-- Large display for current system time -->
        <div class="time-display" id="system-time">--:--:--</div>
      </div>
      
      <!-- Scheduled Action Configuration Section -->
      <div class="section">
        <h2>Scheduled Action</h2>
        <!-- Display current scheduled time -->
        <div class="scheduled-time" id="scheduled-time">--:--</div>
        <!-- Hour input -->
        <div class="form-group">
          <label for="action-hour">Hour (0-23):</label>
          <input type="number" id="action-hour" min="0" max="23" value="12">
        </div>
        <!-- Minute input -->
        <div class="form-group">
          <label for="action-minute">Minute (0-59):</label>
          <input type="number" id="action-minute" min="0" max="59" value="30">
        </div>
        <!-- Save button for scheduled time -->
        <button class="btn" onclick="saveScheduledTime()">Save Scheduled Time</button>
      </div>
      
      <!-- WiFi Networks Configuration Section -->
      <div class="section">
        <h2>WiFi Networks</h2>
        <!-- Container for network list (populated by JavaScript) -->
        <div id="networks-list">
        </div>
        <!-- Action buttons for network management -->
        <button class="btn" onclick="addNetwork()">Add Network</button>
        <button class="btn" onclick="saveNetworks()">Save All Networks</button>
      </div>
    </div>
  </div>

  <script>
    // Global array to store network configurations
    var networks = [];
    
    // Initialize the page when it loads
    window.onload = function() {
      loadStatus();   // Load system status
      loadConfig();   // Load scheduled action configuration
      loadNetworks(); // Load WiFi networks
      // Refresh status every 5 seconds
      setInterval(loadStatus, 5000);
    };
    
    /**
     * Load and display system status information
     * Called every 5 seconds to keep data current
     */
    function loadStatus() {
      fetch('/api/status')
        .then(function(response) { return response.json(); })
        .then(function(data) {
          // Update WiFi status display
          document.getElementById('wifi-status').textContent = data.wifiConnected ? 'Connected' : 'Disconnected';
          // Update IP address display
          document.getElementById('ip-address').textContent = data.ipAddress || 'Not assigned';
          // Update uptime display
          document.getElementById('uptime').textContent = formatUptime(data.uptime);
          // Update memory display
          document.getElementById('free-memory').textContent = formatMemory(data.freeHeap);
          
          // Update system time display
          var time = data.systemTime;
          var timeStr = padZero(time.hour) + ':' + padZero(time.minute) + ':' + padZero(time.second);
          document.getElementById('system-time').textContent = timeStr;
        });
    }
    
    /**
     * Load scheduled action configuration from server
     * Updates the input fields and display
     */
    function loadConfig() {
      fetch('/api/config')
        .then(function(response) { return response.json(); })
        .then(function(data) {
          // Update input fields with current values
          document.getElementById('action-hour').value = data.actionHour;
          document.getElementById('action-minute').value = data.actionMinute;
          // Update scheduled time display
          var scheduledStr = padZero(data.actionHour) + ':' + padZero(data.actionMinute);
          document.getElementById('scheduled-time').textContent = scheduledStr;
        });
    }
    
    /**
     * Load WiFi networks configuration from server
     * Populates the networks array and renders the UI
     */
    function loadNetworks() {
      fetch('/api/networks')
        .then(function(response) { return response.json(); })
        .then(function(data) {
          networks = data.networks || [];
          renderNetworks(); // Update the UI
        });
    }
    
    /**
     * Render the WiFi networks UI
     * Creates input fields for up to 5 networks
     */
    function renderNetworks() {
      var container = document.getElementById('networks-list');
      container.innerHTML = ''; // Clear existing content
      
      // Create UI for up to 5 network slots
      for (var i = 0; i < 5; i++) {
        // Get network data or create empty network object
        var network = networks[i] || {ssid: '', password: '', enabled: false};
        
        // Create network item container
        var networkDiv = document.createElement('div');
        networkDiv.className = 'network-item';
        
        // Determine status indicator class and checkbox state
        var statusClass = network.enabled ? '' : ' disabled';
        var checkedAttr = network.enabled ? ' checked' : '';
        
        // Show delete button only if network has data
        var deleteButton = (network.ssid || network.password) ? 
          '<button class="btn-delete" onclick="deleteNetwork(' + i + ')">Delete</button>' : 
          '<span></span>';
        
        // Build the HTML for this network item
        networkDiv.innerHTML = 
          '<div class="network-header">' +
            '<strong>Network ' + (i + 1) + '</strong>' +
            '<div class="status-indicator' + statusClass + '"></div>' +
          '</div>' +
          '<div class="network-controls">' +
            '<input type="text" placeholder="SSID" value="' + network.ssid + '" onchange="updateNetwork(' + i + ', \'ssid\', this.value)">' +
            '<input type="password" placeholder="Password" value="' + network.password + '" onchange="updateNetwork(' + i + ', \'password\', this.value)">' +
            '<label><input type="checkbox"' + checkedAttr + ' onchange="updateNetwork(' + i + ', \'enabled\', this.checked)"> Enable</label>' +
            deleteButton +
          '</div>';
        
        container.appendChild(networkDiv);
      }
    }
    
    /**
     * Update a specific field of a network configuration
     * @param {number} index - Network index (0-4)
     * @param {string} field - Field to update ('ssid', 'password', 'enabled')
     * @param {*} value - New value for the field
     */
    function updateNetwork(index, field, value) {
      // Create network object if it doesn't exist
      if (!networks[index]) {
        networks[index] = {ssid: '', password: '', enabled: false};
      }
      // Update the specified field
      networks[index][field] = value;
      
      // Re-render the networks after a short delay to show visual changes
      setTimeout(function() { renderNetworks(); }, 100);
    }
    
    /**
     * Delete a network configuration
     * @param {number} index - Network index to delete
     */
    function deleteNetwork(index) {
      if (confirm('Are you sure you want to delete this network?')) {
        // Clear the network data
        networks[index] = {ssid: '', password: '', enabled: false};
        
        // Save changes to server immediately
        var validNetworks = [];
        for (var i = 0; i < networks.length; i++) {
          var n = networks[i];
          if (n && n.ssid && n.ssid.trim() !== '') {
            validNetworks.push(n);
          }
        }
        
        // Send delete request to server
        fetch('/api/networks', {
          method: 'POST',
          headers: {
            'Content-Type': 'application/json'
          },
          body: JSON.stringify({networks: validNetworks})
        })
        .then(function(response) { return response.json(); })
        .then(function(data) {
          if (data.success) {
            console.log('Network deleted successfully');
            loadNetworks(); // Reload from server to ensure consistency
          } else {
            alert('Error deleting network');
            loadNetworks(); // Reload to restore previous state
          }
        })
        .catch(function(error) {
          alert('Error deleting network');
          loadNetworks(); // Reload to restore previous state
        });
      }
    }
    
    /**
     * Add a new network configuration
     * Finds the first empty slot and adds a new network
     */
    function addNetwork() {
      for (var i = 0; i < 5; i++) {
        if (!networks[i] || (!networks[i].ssid && !networks[i].password)) {
          networks[i] = {ssid: 'New Network', password: '', enabled: false};
          renderNetworks();
          break;
        }
      }
    }
    
    /**
     * Save the scheduled action time to the server
     * Validates input and sends POST request
     */
    function saveScheduledTime() {
      var hour = parseInt(document.getElementById('action-hour').value);
      var minute = parseInt(document.getElementById('action-minute').value);
      
      var config = {
        actionHour: hour,
        actionMinute: minute
      };
      
      // Send configuration to server
      fetch('/api/config', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json'
        },
        body: JSON.stringify(config)
      })
      .then(function(response) { return response.json(); })
      .then(function(data) {
        if (data.success) {
          alert('Scheduled time saved successfully!');
          loadConfig(); // Reload to show updated values
        } else {
          alert('Error saving configuration');
        }
      });
    }
    
    /**
     * Save all WiFi networks to the server
     * Filters out empty networks and sends valid ones
     */
    function saveNetworks() {
      // Filter out empty networks (networks without SSID)
      var validNetworks = [];
      for (var i = 0; i < networks.length; i++) {
        var n = networks[i];
        if (n && n.ssid && n.ssid.trim() !== '') {
          validNetworks.push(n);
        }
      }
      
      // Send networks to server
      fetch('/api/networks', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json'
        },
        body: JSON.stringify({networks: validNetworks})
      })
      .then(function(response) { return response.json(); })
      .then(function(data) {
        if (data.success) {
          alert('WiFi networks saved successfully!');
          loadNetworks(); // Reload from server
        } else {
          alert('Error saving networks');
        }
      });
    }
    
    /**
     * Format uptime seconds into human-readable format
     * @param {number} seconds - Uptime in seconds
     * @returns {string} Formatted uptime (e.g., "1h 23m 45s")
     */
    function formatUptime(seconds) {
      var hours = Math.floor(seconds / 3600);
      var minutes = Math.floor((seconds % 3600) / 60);
      var secs = seconds % 60;
      return hours + 'h ' + minutes + 'm ' + secs + 's';
    }
    
    /**
     * Format memory bytes into kilobytes
     * @param {number} bytes - Memory in bytes
     * @returns {string} Formatted memory (e.g., "45 KB")
     */
    function formatMemory(bytes) {
      return Math.round(bytes / 1024) + ' KB';
    }
    
    /**
     * Add leading zero to single-digit numbers
     * @param {number} num - Number to pad
     * @returns {string} Zero-padded string (e.g., "05")
     */
    function padZero(num) {
      return num < 10 ? '0' + num : num.toString();
    }
  </script>
</body>
</html>
)HTML";

#endif // WEB_PAGE_H