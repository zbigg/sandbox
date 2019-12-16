var swarm = require("discovery-swarm");

async function main() {
  const sw = swarm();

  sw.listen(process.env.PORT || 9000);
  sw.join("alibaba-44"); // can be any id/name/hash

  sw.on("connection", function(connection, info) {
    console.log("found + connected to peer", info);
  });
}

main().catch(error => {
  console.error("Failed #1:", error);
  process.exit(2);
});
