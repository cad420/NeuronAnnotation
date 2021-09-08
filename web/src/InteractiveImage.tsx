import { message } from "antd";
import { OmitProps } from "antd/lib/transfer/ListBody";
import React, { useEffect, useRef, useState } from "react";
import TrackballControl from "./FirstPersonController";

const EXP = 0.0001;
function hasDiff(prev: number[], next: number[]): boolean {
  for (let i = 0; i < 3; i++) {
    if (Math.abs(prev[i] - next[i]) > EXP) return true;
  }

  return false;
}

const DEBOUNCE = 5;

const Image: React.FC = (props) => {
  const [src, setSrc] = useState("");
  const [width, setWidth] = useState(600);
  const [height, setHeight] = useState(350);
  const [recording, setRecording] = useState(false); //标注模式
  const img = useRef<HTMLImageElement>(null);
  useEffect(() => {
    let loop = 0;
    let lastPosition = [8617, 8500, 2200];
    let lastTarget = [0, 0, -1];
    let lastUp = [0, 1, 0];
    let lastZoom = 20.0;
    const ws = new WebSocket("ws://127.0.0.1:12121/render");
    ws.binaryType = "arraybuffer";
    ws.onopen = () => {
      console.log("连接渲染服务成功");
      if (!img.current) {
        return;
      }

      const control = new TrackballControl(img.current);
      control.position = [...lastPosition] as [number, number, number];
      control.front = [...lastTarget] as [number, number, number];
      control.up = [...lastUp] as [number, number, number];
      control.rotateSpeed = 50.0;
      control.moveSpeed = 1.0;
      control.zoom = lastZoom;
      control.onRecordStart = () => {
        setRecording(true);
      };
      control.onRecord = (x, y, end) => {
        if (end) {
          setRecording(false);
        }
        console.log("tool: props.selectedTool",props.selectedTool);
        ws.send(
          JSON.stringify({
            click: {
              x,
              y,
            },
            tool: props.selectedTool
          })
        );
      };
      
      ws.send(
        JSON.stringify({
          camera: {
            pos: lastPosition,
            front: lastTarget,
            up: lastUp,
            // zoom: lastZoom,
            zoom:lastZoom,
            n:1,
            f:512
          },
        })
      );

      let i = 0;
      const tick = () => {
        control.move();
        if (++i > DEBOUNCE) {
          i = 0;
          const { position, up, front, zoom } = control;
          const newPosition = [...position];
          const newTarget = [...front];
          const newUp = [...up];
          if (
            hasDiff(newPosition, lastPosition) ||
            hasDiff(newTarget, lastTarget) ||
            hasDiff(newUp, lastUp) ||
            Math.abs(lastZoom - zoom) > EXP
          ) {
            lastPosition = [...newPosition];
            lastTarget = [...newTarget];
            lastUp = [...newUp];
            lastZoom = zoom;
            ws.send(
              JSON.stringify({
                camera: {
                  pos: newPosition,
                  front: newTarget,
                  up: newUp,
                  zoom:zoom,
                  n:1,
                  f:512
                },
              })
            );
          }
        }
        window.requestAnimationFrame(tick);
      };
      loop = window.requestAnimationFrame(tick);
    };
    ws.onmessage = (msg) => {
      const { data } = msg;
      if (typeof msg.data === "object") {
        const bytes = new Uint8Array(msg.data);
        const blob = new Blob([bytes.buffer], { type: "image/jpeg" });
        const url = URL.createObjectURL(blob);
        setSrc(url);
        props.setSrc(url);
        return;
      }  
      try {
        const obj = JSON.parse(data);
        if( obj.type == "error" ){
          console.log(obj.message);
          message.error( obj.message );
        }else if( obj.type == "success" ){
          console.log(obj.message);
          message.success( obj.message );
        }else{
          console.log(obj);
          let p = new Promise(resolve =>{
            resolve(props.setData(obj));
          }).then(()=>{
            props.initSelectedKey();
          })
          //console.log(obj.error);
        }
      } catch {
        console.log(data);
      }
    };
    ws.onerror = () => {
      console.error("连接渲染服务出现错误");
    };

    return () => {
      ws.close();
      window.cancelAnimationFrame(loop);
    };
  }, []);
  {/* <div>{recording ? "已进入标注模式，右键添加标注点" : "按R进入标注模式，F键退出标注模式"}</div> */}
  let finalW, finalH;
  const getWidthAndHeight = function(clientW: number = 1200, ClientH: number = 700) {
      const rate = 0.98;
      const w = clientW * rate;
      const h = ClientH * rate;
      const h1 = w * 700 / 1200;
      let finalW, finalH;
      if (h < h1) {
          [finalW, finalH] = [h * 1200 / 700, h];
      } else {
          [finalW, finalH] = [w, w * 700 / 1200];
      }
      return [finalW, finalH];
  }

  const [w1, setw1] = useState(0);
  const [h1, seth1] = useState(0);

  const w = document.querySelector('.image-wrapper')?.clientWidth;
  const h = document.querySelector('.image-wrapper')?.clientHeight;
  [finalW, finalH] = getWidthAndHeight(w, h);
  

  useEffect(() => {
    const w = document.querySelector('.image-wrapper')?.clientWidth;
    const h = document.querySelector('.image-wrapper')?.clientHeight;
    [finalW, finalH] = getWidthAndHeight(w, h);
    console.log(finalW, finalH);
    setw1(finalW);
    seth1(finalH);
  }, [props.vis]);

  return (
    <div className="image-wrapper">
      <img
        className="interactive-img"
        src={props.src}
        width={w1 || finalW}
        height={h1 || finalH}
        ref={img}
        alt=""
        style={{
          width: `${w1 || finalW}px`,
          height: `${h1 || finalH}px`,
          opacity: props.src ? 1 : 0
        }}
      />
    </div>
  );
};

export default Image;
