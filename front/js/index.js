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

    // //获取元素
    // var txt = document.querySelector('#txt');
    // var btn = document.querySelector('#add');
    // var ul = document.querySelector('.notes');
    // var colors = ['#ef9a9a', '#F48FB1', '#CE93D8', '#B39DDB', '#9FA8DA', '#90CAF9', '#81D4FA', '#80DEEA', '#80CBC4', '#A5D6A7', '#C5E1A5', '#FFCC80', '#FFAB91', '#BCAAA4', '#B0BEC5'];
    // //注册事件
    // btn.onclick = function () {
    //     txt.focus();
    //     if (ul.children.length < 8 && txt.value != '') {

    //         //创建元素
    //         var li = document.createElement('li');

    //         li.style.backgroundColor = colors[parseInt(Math.random() * (colors.length - 1))];
    //         //添加元素
    //         ul.appendChild(li);
    //         li.innerHTML = txt.value + "<p>" + h + ":" + m + " " + ampm + "</p>" + "<a href='javascript:;' style='background:" + li.style.backgroundColor + "' >Delete</a>";
    //         txt.value = '';
    //         txt.focus();
    //         //删除元素
    //         var as = document.querySelectorAll('a');
    //         for (var i = 0; i < as.length; i++) {
    //             as[i].onclick = function () {
    //                 ul.removeChild(this.parentNode);
    //                 txt.focus();
    //             }
    //         }

    //     } else if (ul.children.length == 8) {
    //         txt.value = '';
    //         txt.setAttribute('placeholder', '只能添加8个便签哦！');
    //         txt.focus();

    //     }
    //     else if (txt.value == '') {
    //         txt.setAttribute('placeholder', '请输入内容...');
    //         txt.focus();
    //     }
    // }

}
//递归调用 每秒调用一次
setInterval("myFunction()", 1000);

function getFocus() {
    document.getElementById('seaid').focus();
}

// codemirror构建代码框
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

var btnn = document.querySelector('#note');
var div = document.querySelector('.btm');
var flag1 = true;
var ifDownload = false;
function addFocus() {
    if (!flag1) {
        div.style.transform = 'translateX(0px)'; // 隐藏Codeblock
        flag1 = true;
        txt.blur();
    } else {
        div.style.transform = 'translateX(1300px)';// 展开Codeblock
        txt.focus();
        flag1 = false;
    }
}
btnn.onclick = function () {
    ifDownload = true;
    addFocus();

}

function keepFocus() {
    if (flag1) {
        div.style.transform = 'translateX(1300px)';
        txt.focus();
    }
}

function clickOpen() {
    ifDownload = false;
    keepFocus();
    openFile();
}

function clickSave() {
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

function clickRun() {
    keepFocus();
    if (ifDownload) {
        // 保存文件
    }
}

function clickTerminal() {
    var shell = require('shelljs');//这里要下载shell插件
    module.export = {
        open: function () {
            //生成run.sh脚本
            var comd = 'echo >> run.sh \#/bin/sh cd ~/Documents/2022_SPRSUM/compiler/myCompiler/src\n make test';
            shell.exec(comd, { silent: true });
            //给脚本增加运行权限
            shell.exec('chmod +x run.sh', { silent: true });
            //打开运行
            shell.exec('open -a Terminal run.sh');
        }
    }
    // var comd = 'echo >> run.sh \#/bin/sh ~/Documents/2022_SPRSUM/compiler/myCompiler/src\n make test';
    // shell.exec(comd, { silent: true });
    // //给脚本增加运行权限
    // shell.exec('chmod +x run.sh', { silent: true });
    // //打开运行
    // shell.exec('open -a Terminal run.sh');
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
    return;
}


// function saveFile() {

//     var strFile = "~/Documents/2022_SPR\&SUM/compiler/myCompiler";
//     var objFSO = new ActiveXObject("Scripting.FileSystemObject");
//     // 检查文件是否存在
//     if (!objFSO.FileExists(strFile)) {
//         // 创建文本文件
//         var objStream = objFSO.CreateTextFile(strFile, true);
//         objStream.Write(document.getElementById("txt").innerText);
//         //document.write("创建文本文件: " + strFile + "<br>");
//         objStream.Close(); // 关闭文件
//         alert("ok");
//     }
//     else {
//         alert("文本文件: " + strFile + "已经存在<br>");
//     }
// }




// function myTime() {
//     let time = new Date();
//     let hh = time.getHours();  //时
//     let mm = time.getMinutes();  //分
//     let ss = time.getSeconds();  //秒
//     // Math.floor() 向下取整
//     document.getElementById("1").innerText = Math.floor(hh / 10);
//     document.getElementById("2").innerText = hh % 10;
//     document.getElementById("4").innerText = Math.floor(mm / 10);
//     document.getElementById("5").innerText = mm % 10;
//     document.getElementById("7").innerText = Math.floor(ss / 10);
//     document.getElementById("8").innerText = ss % 10;
// }
// // 一秒执行一次
// setInterval(myTime, 1000);

//兼容不同浏览器获取行间样式
// function getStyle(obj, attr) {
//     return obj.currentStyle ? obj.currentStyle[attr] : getComputedStyle(obj)[attr];
// }

// window.onload = function () {
//     //第一步先追加多个div
//     var oImg = document.getElementById("img");
//     //添加点击事件
//     oImg.οnclick = function () {
//         //先获取图片自身所在的left值
//         var pos = parseFloat(getStyle(oImg, "left"));

//         var arr = [];
//         var num = 0;
//         var timer = null;
//         //抖动频率de数组，一正一负[20,-20,18,-18...]
//         for (var i = 20; i > 0; i -= 2) {
//             arr.push(i, -i)
//         }
//         arr.push(0);
//         //启用定时器前先关闭定时器
//         clearInterval(timer);

//         timer = setInterval(function () {
//             //让图片的left跟随数组里的值变化就会实现左右移动效果
//             oImg.style.left = pos + arr[num] + "px";

//             num++;
//             if (num === arr.length) {
//                 clearInterval(timer);
//             }
//         }, 50)

//     };

// };