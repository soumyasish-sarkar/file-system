const express = require("express");
const http = require("http");
const socketIo = require("socket.io");
const { spawn } = require("child_process");
const path = require("path");
const { exec } = require("child_process");


const app = express();
const server = http.createServer(app);
const io = socketIo(server);

app.use(express.static(path.join(__dirname, "public")));

// Update this to your actual sudo password
const SUDO_PASSWORD = "saumya";

// Universal executor
const executeCommand = (command, callback) => {
  const sudo = spawn("sudo", ["-S", "bash", "-c", command]);

  let output = "";
  let error = "";

  sudo.stdout.on("data", (data) => {
    output += data.toString();
  });

  sudo.stderr.on("data", (data) => {
    error += data.toString();
  });

  sudo.on("close", (code) => {
    if (code !== 0 || error.includes("Permission denied")) {
      callback(`Error: ${error}`);
    } else {
      callback(output || "Success");
    }
  });

  // Send password to sudo prompt
  sudo.stdin.write(`${SUDO_PASSWORD}\n`);
  sudo.stdin.end();
};

io.on("connection", (socket) => {
  console.log("User connected");

  // Create file (touch)
  socket.on("create-file", (fileName) => {
    executeCommand(`touch /mount/fs/${fileName}`, (response) => {
      response.startsWith("Error")
        ? socket.emit("error", response)
        : socket.emit("success", `${fileName} created successfully.`);
    });
  });

  // Create directory (mkdir)
  socket.on("create-dir", (dirName) => {
    executeCommand(`mkdir /mount/fs/${dirName}`, (response) => {
      response.startsWith("Error")
        ? socket.emit("error", response)
        : socket.emit("success", `${dirName} directory created successfully.`);
    });
  });

  // Create hard link
  socket.on("create-hardlink", (target, link) => {
    executeCommand(`ln /mount/fs/${target} /mount/fs/${link}`, (response) => {
      response.startsWith("Error")
        ? socket.emit("error", response)
        : socket.emit("success", `Hard link ${link} created successfully.`);
    });
  });

  // Create symbolic link
  socket.on("create-symlink", (target, link) => {
    executeCommand(`ln -s /mount/fs/${target} /mount/fs/${link}`, (response) => {
      response.startsWith("Error")
        ? socket.emit("error", response)
        : socket.emit("success", `Symbolic link ${link} created successfully.`);
    });
  });

  // Read file (cat)
  socket.on("read-file", (fileName) => {
    executeCommand(`cat /mount/fs/${fileName}`, (response) => {
      response.startsWith("Error")
        ? socket.emit("error", response)
        : socket.emit("file-read", { fileName, content: response });
    });
  });
  
  socket.on("get-kernel-logs", () => {
    exec("sudo dmesg | grep file_system", (err, stdout, stderr) => {
      if (err) {
        socket.emit("kernel-logs", "Error fetching logs.");
      } else {
        socket.emit("kernel-logs", stdout || "No relevant logs found.");
      }
    });
  });

  socket.on("check-permissions", (name) => {
  executeCommand(`ls -ld /mount/fs/${name}`, (response) => {
    if (response.startsWith("Error")) {
      socket.emit("permission-result", `${response}`);
    } else {
      socket.emit("permission-result", `${response}`);
    }
  });
});


// On the server:
socket.on("login", ({ user, pass }, callback) => {
  if (user === "soumyasish_sarkar" && pass === "1234") {
    callback({ success: true });
  } else {
    callback({ success: false });
  }
});




// Write file (only if it exists and is not a directory)
socket.on("write-file", ({ fileName, content }) => {
  const safeContent = content.replace(/"/g, '\\"');
  const filePath = `/mount/fs/${fileName}`;
  const command = `
    if [ -f "${filePath}" ]; then
      echo "${safeContent}" > "${filePath}";
    elif [ -d "${filePath}" ]; then
      echo "Error: '${fileName}' is a directory." >&2;
      exit 1;
    else
      echo "Error: File '${fileName}' does not exist." >&2;
      exit 1;
    fi
  `;

  executeCommand(command, (response) => {
    response.startsWith("Error")
      ? socket.emit("error", response.trim())
      : socket.emit("success", `Written to ${fileName} successfully.`);
  });
});




  // Refresh directory
  socket.on("refresh-dir", () => {
    executeCommand("ls -li /mount/fs", (response) => {
      response.startsWith("Error")
        ? socket.emit("error", response)
        : socket.emit("dir-updated", response);
    });
  });

  // Delete a file
// Delete a file
socket.on("deleteFile", ({ name }) => {
  const filePath = `/mount/fs/${name}`;
  const command = `
    if [ ! -e "${filePath}" ]; then
      echo "Error: '${name}' does not exist." >&2
      exit 1
    elif [ -d "${filePath}" ]; then
      echo "Error: '${name}' is a directory, not a file." >&2
      exit 1
    else
      rm "${filePath}" && echo "Deleted"
    fi
  `;

  executeCommand(command, (response) => {
    const trimmed = response.trim();
    if (trimmed === "Deleted") {
      socket.emit("success", `File '${name}' deleted successfully.`);
    } else {
      socket.emit("error", trimmed);
    }

    socket.emit("refresh-dir");
  });
});



// Delete a directory (only if it's empty)
socket.on("deleteDirectory", ({ name }) => {
  const dirPath = `/mount/fs/${name}`;
  const command = `
    if [ ! -e "${dirPath}" ]; then
      echo "Error: '${name}' does not exist." >&2
      exit 1
    elif [ ! -d "${dirPath}" ]; then
      echo "Error: '${name}' is not a directory." >&2
      exit 1
    elif [ "$(ls -A "${dirPath}")" ]; then
      echo "Error: Directory '${name}' is not empty." >&2
      exit 1
    else
      rmdir "${dirPath}" && echo "Deleted"
    fi
  `;

  executeCommand(command, (response) => {
    const trimmed = response.trim();
    if (trimmed === "Deleted") {
      socket.emit("success", `Directory '${name}' deleted successfully.`);
    } else {
      socket.emit("error", trimmed);
    }

    socket.emit("refresh-dir");
  });
});




  socket.on("disconnect", () => {
    console.log("User disconnected");
  });
});

server.listen(3000, () => {
  console.log("Server running on http://localhost:3000");
});
