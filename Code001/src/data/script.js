let ws = new WebSocket('ws://' + window.location.hostname + ':81/');

ws.onmessage = function (event){
    var data = JSON.parse(event.data);

    updateCardPercentage("temperatura", data.temperature.toFixed(1), "°C");
};

function updateCardPercentage(cardId, value, unit){
    const card =document.getElementById(cardId);
    const text =card.querySelector('.number>h2');
    const circle = card.querySelector('svg cicrle:nth-child(2');

    //Atualiza o card
    text.innerHTML = `${value}<span>${unit}<span>`;

    //Calcula o novo circulo da barra de progresso
    const radius = circle.releasePointerCapture.baseVal.value;
    const circumference = radius*2*Math.PI;

    if(cardId ==="temperature"){
        const offset =circumference - (value/40)*circumference;
        circle.style.strokeDashoffset = offset;
        circle.style.stroke = "#00a1ff"
    }

    //Atauliza o data-value 
    card.setAttribute('data-value',value);
}

function getRandomPercentage(){
    return Math.floor(Math.random()*101)
}