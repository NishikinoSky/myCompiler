// 获取所有.item元素
let items = document.querySelectorAll(".item");
// 设置当前选中项样式的方法
function setActive() {
    // 遍历所有.item元素,移除active样式
    items.forEach((item) => {
        item.classList.remove("active");
    })
    // 为当前选中项添加active样式
    this.classList.add("active");
}
// 遍历所有.item元素,分别为其设置点击事件
items.forEach((item) => {
    item.addEventListener("click", setActive);
})

//创建数组存放背景url
var bgs = new Array('url("images/background/sky.png")', 'url("images/background/night.png")');
//插入背景函数
function insertBg() {
    var now = new Date();
    var hour = now.getHours();
    if (hour >= 6 && hour < 18) {
        document.getElementById('bgid').style.backgroundImage = bgs[0];
    } else if ((hour >= 18 && hour < 24) || (hour >= 0 && hour < 6)) {
        document.getElementById('bgid').style.backgroundImage = bgs[1];
    }
}

//创建计时器
function myFunction() {
    var time = new Date();
    y = time.getFullYear();
    mon = time.getMonth() + 1;
    d = time.getDate();
    var h = time.getHours();
    var ampm = h < 12 ? 'AM' : 'PM';
    if (h < 10) {
        h = '0' + h;
    }
    else if (h >= 12 && h < 22) {
        h = '0' + (h % 12)
    } else if (h >= 22) {
        h = h % 12;
    }
    else {
        h = h;
    }

    var m = time.getMinutes();
    m = m < 10 ? '0' + m : m;

    var s = time.getUTCSeconds();
    s = s < 10 ? '0' + s : s;


    var wArr = ["Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"];
    w = wArr[time.getDay()];
    //插入顶部时间
    document.getElementById("rtbox").innerHTML = y + "-" + mon + "-" + d + " " + w;
    document.getElementById("lttime").innerHTML = ampm + ' ' + h + ":" + m + ":" + s;
}
//递归调用 每秒调用一次
setInterval("myFunction()", 1000);

function getFocus() {
    document.getElementById('seaid').focus();
}

// codeblock codemirror构建代码框
var editor = CodeMirror.fromTextArea(document.getElementById("txt"), {
    mode: "text/x-java", //实现Java代码高亮
    mode: "text/x-c++src",
    lineNumbers: true,//是否显示每一行的行数
    indentUnit: 4, //缩进单位为4
    matchBrackets: true, //括号匹配
    styleActiveLine: true, //当前行背景高亮
    readOnly: false,//只读
    lineWrapping: true,	//代码折叠
    foldGutter: true,
    smartIndent: true,//智能缩进
    gutters: ["CodeMirror-linenumbers", "CodeMirror-foldgutter"],
    lineWrapping: true, //自动换行
    theme: 'ayu-mirage', //编辑器主题
});
//var height = 650;
editor.setSize('100%', '100%');

// resultblock codemirror构建代码框
var resultEdi = CodeMirror.fromTextArea(document.getElementById("result"), {
    mode: "text/x-java", //实现Java代码高亮
    mode: "text/x-c++src",
    lineNumbers: true,//是否显示每一行的行数
    indentUnit: 4, //缩进单位为4
    matchBrackets: true, //括号匹配
    styleActiveLine: true, //当前行背景高亮
    readOnly: true,//只读
    lineWrapping: true,	//代码折叠
    foldGutter: true,
    smartIndent: true,//智能缩进
    gutters: ["CodeMirror-linenumbers", "CodeMirror-foldgutter"],
    lineWrapping: true, //自动换行
    theme: 'ayu-mirage', //编辑器主题
});
//var height = 650;
resultEdi.setSize('100%', '100%');

var div = document.querySelector('.btm');
var divT = document.querySelector('.res');
var flag = true;
var resFlag = true;
var ifDownload = false;
function addFocus() {
    if (!flag) {
        div.style.transform = 'translateX(0px)'; // 隐藏Codeblock
        flag = true;
        txt.blur();
    } else {
        div.style.transform = 'translateX(1300px)';// 展开Codeblock
        txt.focus();
        flag = false;
    }
    if (!resFlag) {
        divT.style.transform = 'translateY(0px)'; // 隐藏Resultblock
        resFlag = true;
        result.blur();
    } else {
        divT.style.transform = 'translateY(-500px)';// 展开Resultblock
        result.focus();
        resFlag = false
    }
}

function keepFocus() {
    if (flag) {
        div.style.transform = 'translateX(1300px)';
        txt.focus();
    }
}

function keepResFocus() {
    if (resFlag) {
        divT.style.transform = 'translateY(-500px)';// 展开Resultblock
        result.focus();
    }
}

function clickOpen() {
    ifDownload = false;
    keepFocus();
    openFile();
}

function clickSave() {
    saveFile();
    ifDownload = true;
}

function clickRun() {
    keepFocus();
    if (!ifDownload) {
        // 保存文件
        saveFile();
        ifDownload = true;
    }
    var httpRequest = new XMLHttpRequest();
    httpRequest.open("POST", "http://localhost:5500/run", true);
    httpRequest.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
    httpRequest.send("source=" + document.getElementById("txt").value);
    httpRequest.onreadystatechange = () => {
        if (httpRequest.readyState == 4 && httpRequest.status == 200) {
            var obj = JSON.parse(httpRequest.responseText);
            console.log(obj.data)
            document.getElementById("txt").value = obj.data;
        }
    }
}


// var term = new Terminal({
//     rendererType: 'canvas',
//     cols: 110,
//     rows: 7,
//     disableStdin: false,
//     cursorBlink: true, //光标闪烁
//     convertEol: true, //启用时，光标将设置为下一行的开头
//     allowTransparency: true,
//     theme: {
//         foreground: "#FFA3B5", //字体
//         background: "#20242f", //背景色
//         cursor: "#cab0fd", //光标颜色
//         lineHeight: 15,
//     }
// });
// term.open(document.getElementById('terminal')); //绑定dom节点
// term.write('Hello from \x1B[1;3;31mDiana\x1B[0m $ ');
// var divT = document.querySelector('.terminalClass');


function clickTerminal() {
    keepResFocus();
    $('#resInput').trigger('click');
    document.getElementById('resInput').addEventListener('change', function selectedFileChanged() {
        if (this.files.length == 0) {
            console.log('Choose file nia~');
            return;
        }
        const reader = new FileReader();
        reader.onload = function fileReadCompleted() {
            //当读取完成时，内容只在`reader.result`中
            console.log(reader.result);
            var content = reader.result;
            resultEdi.setValue(content);
        };
        reader.readAsText(this.files[0]);
    });
}

function openFile() {
    $('#fileInput').trigger('click');
    document.getElementById('fileInput').addEventListener('change', function selectedFileChanged() {
        if (this.files.length == 0) {
            console.log('Choose file nia~');
            return;
        }
        const reader = new FileReader();
        reader.onload = function fileReadCompleted() {
            //当读取完成时，内容只在`reader.result`中
            console.log(reader.result);
            var content = reader.result;
            editor.setValue(content);
        };
        reader.readAsText(this.files[0]);
    });
}


function saveFile() {
    var content = editor.getValue();
    var blob = new Blob([content], { type: "text/plain;charset=utf-8" });
    //创建一个blob的对象连链接
    var url = window.URL.createObjectURL(blob);
    // 创建一个链接元素，是属于 a 标签的链接元素
    var link = document.createElement('a');
    // 把blob的对象链接赋值给新创建的这个 a 链接
    link.href = url;
    // 设置下载的属性（所以使用的是download），这个是a标签的一个属性
    link.setAttribute('download', "test.cmm");
    link.click();
}