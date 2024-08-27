const ctx = document.getElementById('barchart').getContext('2d');
const barchart = new Chart(ctx, {
    type: 'bar',
    data: {
        labels: [],
        datasets: [{
            label: 'Keterangan Suhu Lingkungan',
            data: [],
            backgroundColor: ['rgba(54, 162, 235, 0.2)',
                 'rgba(255, 99, 132, 0.2)',
                'rgba(54, 162, 235, 0.2)',
                'rgba(255, 206, 86, 0.2)',
                'rgba(75, 192, 192, 0.2)',
                'rgba(153, 102, 255, 0.2)',
                'rgba(255, 159, 64, 0.2)',],
            borderColor: 'rgba(54, 162, 235, 1)',
            borderWidth: 1
        }]
    },
    options: {
        scales: {
            y: {
                beginAtZero: true
            }
        }
    }
});

async function fetchData() {
    const apiKey = '2WEDLR29SB1UFFFI'; // Ganti dengan API Key dari ThingSpeak Anda
    const channelId = '2610736'; // Ganti dengan Channel ID dari ThingSpeak Anda
    const url = `https://api.thingspeak.com/channels/${channelId}/feeds.json?api_key=${apiKey}&results=10`;

    try {
        const response = await fetch(url);
        const data = await response.json();
        const feeds = data.feeds;

        const labels = feeds.map(feed => new Date(feed.created_at).toLocaleString());
        const suhuData = feeds.map(feed => parseFloat(feed.field1));

        barchart.data.labels = labels;
        barchart.data.datasets[0].data = suhuData;
        barchart.update();
    } catch (error) {
        console.error('Error fetching data from ThingSpeak:', error);
    }
}

setInterval(fetchData, 15000); // Mengambil data setiap 15 detik
fetchData(); // Mengambil data saat halaman dimuat
