const socket = io();

function updateStatus(message, isError = false) {
  const statusBox = document.getElementById("status-message");
  statusBox.textContent = (isError ? "❌ " : "✅ ") + message;
  statusBox.style.color = isError ? "red" : "green";
}

function createFile() {
  const fileName = document.getElementById("file-name").value;
    if (!fileName) return updateStatus("File name cannot be empty", true);

  socket.emit("create-file", fileName);
}


function createDir() {
  const dirName = document.getElementById("dir-name").value;
    if (!dirName) return updateStatus("Directory name cannot be empty", true);
  socket.emit("create-dir", dirName);
}

function createHardLink() {
  const target = document.getElementById("hardlink-target").value;
  const linkName = document.getElementById("hardlink-name").value;
  socket.emit("create-hardlink", target, linkName);
}

function createSymlink() {
  const target = document.getElementById("symlink-target").value;
  const linkName = document.getElementById("symlink-name").value;
  socket.emit("create-symlink", target, linkName);
}

function refreshDir() {
  socket.emit("refresh-dir");
}

function readFile() {
  const fileName = document.getElementById("read-file-name").value;
  socket.emit("read-file", fileName);
}


function writeFile() {
  const fileName = document.getElementById("write-file-name").value;
  const content = document.getElementById("file-content").value;
  socket.emit("write-file", { fileName, content });
}

function deleteFile() {
  const fileName = document.getElementById("delete-file-name").value.trim();
  if (!fileName) return alert("Please enter a file name to delete.");
  socket.emit("deleteFile", { name: fileName });
}

function deleteDirectory() {
  const dirName = document.getElementById("delete-dir-name").value.trim();
  if (!dirName) return alert("Please enter a directory name to delete.");
  socket.emit("deleteDirectory", { name: dirName });
}

// Directory Content
socket.on("dir-updated", (dirContent) => {
  document.getElementById("dir-content").textContent = dirContent;
});

// File Read
socket.on("file-read", ({ fileName, content }) => {
  document.getElementById("read-output").textContent = content || "(Empty file)";
  updateStatus(`Read from ${fileName} successful.`);
});

// File Created
socket.on("file-created", (fileName) => {
  updateStatus(`${fileName} has been created.`);
  refreshDir();
});

// Directory Created
socket.on("dir-created", (dirName) => {
  updateStatus(`${dirName} directory created.`);
  refreshDir();
});

// Hard Link Created
socket.on("hardlink-created", (linkName) => {
  updateStatus(`Hard link ${linkName} created.`);
  refreshDir();
});

// Symlink Created
socket.on("symlink-created", (linkName) => {
  updateStatus(`Symbolic link ${linkName} created.`);
  refreshDir();
});

// General Success
socket.on("success", (message) => {
  updateStatus(message);
});

// General Error
socket.on("error", (message) => {
  updateStatus(message, true);
});

// Fetch logs from dmesg | grep "file_system"
function fetchLogs() {
  socket.emit("get-kernel-logs");
}


socket.on("kernel-logs", (logData) => {
  document.getElementById("status-message").textContent = logData;
});

// Call this every 5 seconds
setInterval(fetchLogs, 5000);

function checkPermissions() {
  const name = document.getElementById("perm-file-name").value.trim();
  if (!name) return updateStatus("Enter file/dir name", true);
  socket.emit("check-permissions", name);
}

socket.on("permission-result", (msg) => {
  updateStatus(msg);
});






