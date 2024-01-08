  const firebaseConfig = {
    apiKey: "AIzaSyDvw9KCqz1IsNQ7FwkQvSPl_jbSg_OPGCY",
    authDomain: "waterflowsensor-f36b6.firebaseapp.com",
    databaseURL: "https://waterflowsensor-f36b6-default-rtdb.firebaseio.com",
    projectId: "waterflowsensor-f36b6",
    storageBucket: "waterflowsensor-f36b6.appspot.com",
    messagingSenderId: "600221830098",
    appId: "1:600221830098:web:bddceb8c4257f242cf560b",
    measurementId: "G-F0MMGWWL0B"
  };

  // Initialize Firebase
  const app = initializeApp(firebaseConfig);
  const analytics = getAnalytics(app);

// Initialize Firebase
firebase.initializeApp(firebaseConfig);

// Reference to your Firebase database
const database = app.database();

// Reference to the water sensor readings
const waterSensorRef = database.ref("/user");

// Update the readings on the web page
waterSensorRef.on('value', (snapshot) => {
    const data = snapshot.val();
	console.log(data);

    // Update HTML elements with sensor readings
   // document.getElementById('balance').innerText = `${data.Balance}`;
   // document.getElementById('totalLiters').innerText = `Total Liters: ${data.totalLiters}`;
    //document.getElementById('totalCubicMeters').innerText = `Total Cubic Meters: ${data.totalCubicMeters}`;
    //document.getElementById('remainingCubicMeters').innerText = `Remaining Cubic Meters: ${data.remainingCubicMeters}`;

    // Add animation logic here (e.g., using CSS animations or JavaScript animations)
});
